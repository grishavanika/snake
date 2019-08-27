#include <vector>
#include <algorithm>
#include <iterator>
#include <random>

#include <cstdlib>
#include <cassert>

enum class State
{
    Start,
    Running,
    Loss,
    Win,
    Quit,
    Pause,
};

struct Direction
{
    int dx = 1;
    int dy = 0;

    static constexpr Direction Up()    { return {0, -1}; }
    static constexpr Direction Down()  { return {0, +1}; }
    static constexpr Direction Left()  { return {-1, 0}; }
    static constexpr Direction Right() { return {+1, 0}; }

    static Direction GetOpposite(Direction d)
    {
        d.dx *= -1;
        d.dy *= -1;
        return d;
    }
};

inline bool operator==(const Direction& lhs, const Direction& rhs)
{
    return (lhs.dx == rhs.dx)
        && (lhs.dy == rhs.dy);
}

inline bool operator!=(const Direction& lhs, const Direction& rhs)
{
    return !(lhs == rhs);
}

enum class HitTarget
{
    None,
    Snake,
    Food,
};

struct Position
{
    int x = 0;
    int y = 0;
};

inline bool operator==(const Position& lhs, const Position& rhs)
{
    return (lhs.x == rhs.x)
        && (lhs.y == rhs.y);
}

inline bool operator!=(const Position& lhs, const Position& rhs)
{
    return !(lhs == rhs);
}

struct Game
{
    explicit Game(unsigned width, unsigned height);

    const Position& head() const;
    State state() const;
    const Position& food() const;
    const std::vector<Position>& parts() const;
    unsigned speed() const;

    void on_update(unsigned t_ms);
    void on_toggle_pause(unsigned t_ms);
    void on_quit();
    void on_reset();

    void try_change_direction(const Direction& d);

private:
    void handle_input(unsigned tile_dt);
    HitTarget on_move(unsigned t_ms);
    State consume_food();
    void pop_direction();

    unsigned increase_speed() const;

    Direction find_tail_direction(
        const Position& before_tail
        , const Position& tail) const;

    Position assist_with_tail_crash(const Position& old_tail
        , const Position& new_tail
        , const Direction& tail_direction) const;

    Position try_eat(Direction current) const;

    bool is_inside_snake(const Position& p
        , unsigned skip_tail = 0) const;

    Position generate_new_food() const;

    unsigned get_move_delta(unsigned t_ms) const;

    Position make_tile_in_direction(Position p, Direction d) const;

    Direction next_direction() const;

private:
    State state_{State::Start};
    std::vector<Position> parts_{};
    Position food_{-1, -1};
    unsigned last_move_time_ms_ = 0;
    unsigned speed_ = 1; // tiles per second
    unsigned field_width_ = 1;
    unsigned field_height_ = 1;
    std::vector<Direction> directions_queue_{};
    Direction direction_;

    mutable std::mt19937 gen_{std::random_device()()};
};

/*explicit*/ inline Game::Game(unsigned width, unsigned height)
{
    field_width_ = width;
    field_height_ = height;
    on_reset();
}

inline const Position& Game::head() const
{
    assert(!parts_.empty());
    return parts_.back();
}

inline State Game::state() const
{
    return state_;
}

inline const Position& Game::food() const
{
    return food_;
}

inline unsigned Game::speed() const
{
    return speed_;
}

inline const std::vector<Position>& Game::parts() const
{
    return parts_;
}

inline void Game::on_update(unsigned t_ms)
{
    if (state_ != State::Running)
    {
        return;
    }

    const unsigned tile_dt = get_move_delta(t_ms);
    handle_input(tile_dt);

    if (tile_dt == 0)
    {
        return;
    }

    last_move_time_ms_ = t_ms;
    const HitTarget hit = on_move(tile_dt);
    switch (hit)
    {
    case HitTarget::None:
        break;
    case HitTarget::Snake:
        state_ = State::Loss;
        break;
    case HitTarget::Food:
        state_ = consume_food();
        if (state_ == State::Running)
        {
            food_ = generate_new_food();
            speed_ = increase_speed();
        }
        break;
    }
}

inline void Game::handle_input(unsigned tile_dt)
{
    const auto next = next_direction();
    if (next == Direction::GetOpposite(direction_))
    {
        pop_direction();
        return;
    }

    direction_ = next;
    if (tile_dt != 0)
    {
        pop_direction();
    }
}

inline HitTarget Game::on_move(unsigned tile_dt)
{
    assert(parts_.size() > 0);
    assert(tile_dt > 0);

    HitTarget hit = HitTarget::None;
    for (unsigned i = 0; i < tile_dt; ++i)
    {
        const Position new_head = make_tile_in_direction(head(), direction_);

        if (is_inside_snake(new_head, i + 1/*skip tails*/))
        {
            hit = HitTarget::Snake;
        }
        else if ((new_head == food_) && (hit != HitTarget::Snake))
        {
            hit = HitTarget::Food;
        }

        parts_.push_back(new_head);
    }

    parts_.erase(std::begin(parts_), std::begin(parts_) + tile_dt);

    return hit;
}

inline unsigned Game::get_move_delta(unsigned t_ms) const
{
    assert(last_move_time_ms_ <= t_ms);
    const unsigned dt = (t_ms - last_move_time_ms_);
    const unsigned tile_dt = static_cast<unsigned>(
        std::round((speed_ * dt) / 1000.f));
    return tile_dt;
}

inline void Game::pop_direction()
{
    if (!directions_queue_.empty())
    {
        directions_queue_.erase(directions_queue_.begin());
    }
}

inline Direction Game::next_direction() const
{
    if (directions_queue_.empty())
    {
        return direction_;
    }
    return directions_queue_[0];
}

inline Position Game::make_tile_in_direction(Position p, Direction d) const
{
    auto wrap = [](int v, int wrapped)
    {
        if (v < 0)
        {
            return (wrapped - 1);
        }
        else if (v >= wrapped)
        {
            return (v % wrapped);
        }
        return v;
    };
    const int x = wrap(p.x + d.dx, field_width_);
    const int y = wrap(p.y + d.dy, field_height_);
    return Position{x, y};
}

inline bool Game::is_inside_snake(const Position& p
    , unsigned skip_tail /*= 0*/) const
{
    assert(parts_.size() >= skip_tail);
    const auto end = std::end(parts_);
    const auto begin = std::begin(parts_) + skip_tail;
    return (std::find(begin, end, p) != end);
}

inline State Game::consume_food()
{
    const Position tail = try_eat(direction_);
    if (is_inside_snake(tail))
    {
        return State::Loss;
    }

    parts_.insert(std::begin(parts_), tail);

    if (parts_.size() >= (field_width_ * field_height_))
    {
        return State::Win;
    }

    return State::Running;
}

inline Position Game::try_eat(Direction current) const
{
    // Insert new tile in the tail.
    // Detect (old) tail direction by looking at last
    // 2 tail's tiles.
    const Direction tail_direction = (parts_.size() >= 2)
        ? find_tail_direction(parts_[1], parts_[0])
        : current;
    const Position old_tail = parts_[0];
    const Position new_tail = make_tile_in_direction(old_tail
        , Direction::GetOpposite(tail_direction));
    const Position fixed = assist_with_tail_crash(
        old_tail, new_tail, tail_direction);
    return fixed;
}

inline Direction Game::find_tail_direction(
    const Position& before_tail
    , const Position& tail) const
{
    // They should be in one line (either horizontal or vertical)
    assert((tail.x == before_tail.x)
        || (tail.y == before_tail.y));

    auto wrap = [](int ds, int max)
    {
        if (ds == (max - 1))
        {
            ds = -1;
        }
        else if (ds == -(max - 1))
        {
            ds = 1;
        }
        assert((ds == 0) || (ds == 1) || (ds == -1));
        return ds;
    };

    Direction d;
    d.dx = wrap(before_tail.x - tail.x, field_width_);
    d.dy = wrap(before_tail.y - tail.y, field_height_);
    return d;
}

inline Position Game::assist_with_tail_crash(const Position& old_tail
    , const Position& new_tail
    , const Direction& tail_direction) const
{
    if (!is_inside_snake(new_tail))
    {
        return new_tail;
    }

    constexpr Direction k_Horizontal[] = {Direction::Left(), Direction::Right()};
    constexpr Direction k_Vertical[]   = {Direction::Up(),   Direction::Down()};

    // When tail moves vertically and has obstacle
    // try to insert in horizontal direction (Left and Right)
    // and vice-versa.
    const Direction* help = (tail_direction.dy != 0)
        ? k_Horizontal : k_Vertical;
    for (int i = 0; i < 2; ++i)
    {
        const Position tail = make_tile_in_direction(old_tail, help[i]);
        if (!is_inside_snake(tail))
        {
            return tail;
        }
    }
    return new_tail;
}

inline Position Game::generate_new_food() const
{
    // Warn: this may take forever when snake is big
    assert(parts_.size() < (field_width_ * field_height_));

    Position food;
    std::uniform_int_distribution<int> w(0, field_width_ - 1);
    std::uniform_int_distribution<int> h(0, field_height_ - 1);
    do
    {
        food.x = w(gen_);
        food.y = h(gen_);
    }
    while (is_inside_snake(food));
    return food;
}

inline unsigned Game::increase_speed() const
{
    constexpr unsigned k_MaxSpeed = 30;
    return (speed_ < k_MaxSpeed)
        ? (speed_ + 1)
        : speed_;
}

inline void Game::try_change_direction(const Direction& d)
{
    if ((state_ != State::Running)
        || (d == next_direction()))
    {
        return;
    }
    directions_queue_.push_back(d);
}

inline void Game::on_quit()
{
    on_reset();
    state_ = State::Quit;
}

inline void Game::on_reset()
{
    state_ = State::Start;

    food_ = {-1, -1};
    last_move_time_ms_ = 0;
    speed_ = 5;

    direction_ = Direction::Right();
    directions_queue_.clear();

    parts_.clear();
    parts_.push_back({});
    parts_.back().x = static_cast<int>(field_width_ / 2);
    parts_.back().y = static_cast<int>(field_height_ / 2);

}

inline void Game::on_toggle_pause(unsigned t_ms)
{
    if (state_ == State::Start)
    {
        on_reset();
        state_ = State::Running;
        food_ = generate_new_food();
        last_move_time_ms_ = t_ms;
    }
    else if (state_ == State::Pause)
    {
        state_ = State::Running;
        last_move_time_ms_ = t_ms;
    }
    else if (state_ == State::Running)
    {
        state_ = State::Pause;
    }
}

#define SDL_MAIN_HANDLED
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>

#include <algorithm>
#include <sstream>
#include <string>

constexpr int k_ScreenWidth  = 480;
constexpr int k_ScreenHeight = 480;
constexpr int k_TileHeight   = 12;
constexpr int k_TileWidth    = 12;

const SDL_Color k_WhiteColor{0xff, 0xff, 0xff, 0xff};
const SDL_Color k_RedColor  {0xff, 0x32, 0x32, 0xff};
const SDL_Color k_GreenColor{0x32, 0xff, 0x32, 0xff};
const SDL_Color k_GrayColor {0x64, 0x64, 0x64, 0xff};

static_assert((k_ScreenWidth % k_TileWidth) == 0,
    "Bad width: tiles count should be integral");
static_assert((k_ScreenHeight % k_TileHeight) == 0,
    "Bad height: tiles count should be integral");

void AbortOnSDLError(int code)
{
    if (code != 0)
    {
        assert(false && "SDL call failed");
        abort();
    }
}

void AbortOnSDLError(const void* resource)
{
    if (!resource)
    {
        assert(false && "SDL call (resource) failed");
        abort();
    }
}

SDL_Rect PositionToTileRect(const Position& p)
{
    SDL_Rect r{};
    r.x = (p.x * k_TileWidth);
    r.y = (p.y * k_TileHeight);
    r.w = k_TileWidth;
    r.h = k_TileHeight;
    return r;
}

std::vector<SDL_Rect> PositionsToTileRects(const std::vector<Position>& parts)
{
    std::vector<SDL_Rect> rs;
    rs.reserve(parts.size());
    std::transform(std::begin(parts), std::end(parts)
        , std::back_inserter(rs)
        , &PositionToTileRect);
    return rs;
}

SDL_Color MakeDarkerColor(const SDL_Color& c, float k = 0.9f)
{
    auto darker = [k](Uint8 v) { return static_cast<Uint8>(v * k); };
    SDL_Color d{};
    d.r = darker(c.r);
    d.g = darker(c.g);
    d.b = darker(c.b);
    d.a = c.a;
    return d;
}

void RenderSnake(SDL_Renderer* renderer
    , const Game& game
    , const SDL_Color& color)
{
    const auto rs = PositionsToTileRects(game.parts());
    assert(!rs.empty());

    AbortOnSDLError(SDL_SetRenderDrawColor(
        renderer, color.r, color.g, color.b, 0xff));
    AbortOnSDLError(SDL_RenderFillRects(
        renderer, rs.data(), static_cast<int>(rs.size())));

    constexpr bool k_DebugDraw = false;
    if (k_DebugDraw)
    {
        const SDL_Color darker = MakeDarkerColor(color, 0.5f);
        AbortOnSDLError(SDL_SetRenderDrawColor(
            renderer, darker.r, darker.g, darker.b, 0xff));
        AbortOnSDLError(SDL_RenderDrawRects(
            renderer, rs.data(), static_cast<int>(rs.size())));
    }
}

void RenderFood(SDL_Renderer* renderer
    , const Position& food
    , const SDL_Color& color)
{
    if ((food.x < 0) || (food.y < 0))
    {
        return;
    }
    const auto r = PositionToTileRect(food);

    AbortOnSDLError(filledCircleRGBA(renderer
        , static_cast<Sint16>(r.x + r.w / 2)
        , static_cast<Sint16>(r.y + r.h / 2)
        , static_cast<Sint16>(std::min(r.w, r.h) / 2)
        , color.r, color.g, color.b, 0xff));
}

void RenderHead(SDL_Renderer* renderer
    , const Position& head
    , const SDL_Color& base_color)
{
    const auto r = PositionToTileRect(head);
    const auto color = MakeDarkerColor(base_color);

    AbortOnSDLError(SDL_SetRenderDrawColor(
        renderer, color.r, color.g, color.b, 0xff));
    AbortOnSDLError(SDL_RenderFillRect(
        renderer, &r));
}

void RenderGame(SDL_Renderer* renderer
    , const Game& game
    , const SDL_Color& color)
{
    RenderFood(renderer, game.food(), color);
    RenderSnake(renderer, game, color);
    RenderHead(renderer, game.head(), color);
}

struct TickData
{
    Game& game;
    SDL_Renderer* renderer = nullptr;

    TTF_Font* font = nullptr;
    SDL_Texture* text = nullptr;
    SDL_Rect text_rect{};
};

void RenderStats(TickData& data)
{
    assert(data.text);

    SDL_Rect dst{};

    dst.x = ((k_ScreenWidth - data.text_rect.w) / 2);
    dst.y = ((k_ScreenHeight - data.text_rect.h) / 2);
    dst.w = data.text_rect.w;
    dst.h = data.text_rect.h;

    AbortOnSDLError(SDL_RenderCopy(data.renderer, data.text
        , &data.text_rect
        , &dst));
}

void RenderAll(TickData& data)
{
    SDL_Renderer* renderer = data.renderer;
    Game& game = data.game;

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0xff);
    SDL_RenderClear(renderer);

    switch (data.game.state())
    {
    case State::Running:
        RenderGame(renderer, game, k_WhiteColor);
        break;
    case State::Start:
    case State::Pause:
        RenderGame(renderer, game, k_GrayColor);
        RenderStats(data);
        break;
    case State::Loss:
        RenderGame(renderer, game, k_RedColor);
        RenderStats(data);
        break;
    case State::Win:
        RenderGame(renderer, game, k_GreenColor);
        RenderStats(data);
        break;
    case State::Quit:
        break;
    }

    SDL_RenderPresent(renderer);
}

SDL_Texture* DrawTextLinesToTexture(
    SDL_Renderer* renderer
    , TTF_Font* font
    , std::vector<std::string> lines
    , SDL_Rect& size
    , SDL_Color color = k_WhiteColor)
{
    size = {};

    SDL_Texture* texture = SDL_CreateTexture(renderer
        , SDL_PIXELFORMAT_RGBA8888
        , SDL_TEXTUREACCESS_TARGET
        , k_ScreenWidth
        , k_ScreenHeight);
    AbortOnSDLError(texture);
    AbortOnSDLError(SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND));
    AbortOnSDLError(SDL_SetRenderTarget(renderer, texture));
    AbortOnSDLError(SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0));
    AbortOnSDLError(SDL_RenderClear(renderer));

    auto draw_next_line = [&](const std::string& s)
    {
        SDL_Surface* surface = TTF_RenderText_Solid(
            font, s.c_str(), color);
        AbortOnSDLError(surface);
        SDL_Texture* text = SDL_CreateTextureFromSurface(renderer, surface);
        AbortOnSDLError(text);

        SDL_Rect dst{};
        dst.x = 0;
        dst.y = size.h;
        dst.w = surface->w;
        dst.h = surface->h;
        size.w = std::max(dst.w, size.w);
        size.h += dst.h;

        SDL_FreeSurface(surface);

        AbortOnSDLError(SDL_RenderCopy(
            renderer, text, nullptr/*copy full texture*/, &dst));

        SDL_DestroyTexture(text);
    };

    for (const std::string& s : lines)
    {
        draw_next_line(s);
    }

    AbortOnSDLError(SDL_SetRenderTarget(renderer, nullptr/*reset to default*/));
    return texture;
}

SDL_Texture* BuildMenu(const Game& game
    , SDL_Renderer* renderer
    , TTF_Font* font
    , SDL_Rect& text_rect)
{
    std::vector<std::string> lines;
    std::ostringstream ss;
    auto new_line = [&]()
    {
        lines.push_back(ss.str());
        ss.str(std::string());
    };

    ss << "Speed : " << game.speed();        new_line();
    ss << "Length: " << game.parts().size(); new_line();
    ss << "Space : " << "toggle pause";      new_line();
    ss << "Esc   : " << "restart";           new_line();

    return DrawTextLinesToTexture(renderer, font, std::move(lines), text_rect);
}

void UpdateMenu(TickData& data, State old_state, bool hard_reset)
{
    const State state = data.game.state();
    const bool changed = (old_state != state);
    
    const bool invalidate_menu = (hard_reset || changed);
    if (invalidate_menu && data.text)
    {
        SDL_DestroyTexture(data.text);
        data.text = nullptr;
    }

    const bool need_text = (state != State::Running) && (state != State::Quit);
    if (need_text && !data.text)
    {
        data.text = BuildMenu(data.game, data.renderer
            , data.font, data.text_rect);
    }
}

void MainTick(void* data_ptr)
{
    TickData* data = static_cast<TickData*>(data_ptr);
    Game& game = data->game;
    SDL_Renderer* renderer = data->renderer;

    const unsigned t_ms = SDL_GetTicks();
    const State old_state = game.state();
    bool hard_menu_reset = false;

    auto handle_key_down = [&](SDL_Keycode code)
    {
        switch (code)
        {
        case SDLK_ESCAPE:
            game.on_reset();
            hard_menu_reset = true;
            break;
        case SDLK_SPACE:
            game.on_toggle_pause(t_ms);
            break;
        case SDLK_UP:
        case SDLK_w:
            game.try_change_direction(Direction::Up());
            break;
        case SDLK_DOWN:
        case SDLK_s:
            game.try_change_direction(Direction::Down());
            break;
        case SDLK_LEFT:
        case SDLK_a:
            game.try_change_direction(Direction::Left());
            break;
        case SDLK_RIGHT:
        case SDLK_d:
            game.try_change_direction(Direction::Right());
            break;
        }
    };

    SDL_Event e{};
    while (SDL_PollEvent(&e))
    {
        switch (e.type)
        {
        case SDL_QUIT:
            game.on_quit();
            break;
        case SDL_KEYDOWN:
            handle_key_down(e.key.keysym.sym);
            break;
        }
    }

    game.on_update(t_ms);
    UpdateMenu(*data, old_state, hard_menu_reset);
    RenderAll(*data);
}

#if (__EMSCRIPTEN__)
#include <emscripten/emscripten.h>

void MainLoop(TickData& data)
{
    emscripten_set_main_loop_arg(&MainTick
        , &data
        , -1  // use whatever FPS browser needs
        , 1); // simulate infinite loop. Don't destroy objects on stack (?)
}

int main(int, char**)
{

#else

#include <Windows.h>
#include <tchar.h>

void MainLoop(TickData& data)
{
    while (data.game.state() != State::Quit)
    {
        MainTick(&data);
    }
}

int WINAPI _tWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPTSTR, _In_ int)
{
    SDL_SetMainReady();
#endif

    // Initialize SDL. Ignore any errors and leak resources
    AbortOnSDLError(SDL_Init(SDL_INIT_VIDEO));
    AbortOnSDLError(TTF_Init());

    SDL_Window* window = SDL_CreateWindow(
        "Snake" // title
        , SDL_WINDOWPOS_CENTERED // x position
        , SDL_WINDOWPOS_CENTERED // y position
        , k_ScreenWidth
        , k_ScreenHeight
        , SDL_WINDOW_SHOWN);
    AbortOnSDLError(window);
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window
        , -1 // first supporting renderer
        , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    AbortOnSDLError(renderer);

    Game game((k_ScreenWidth / k_TileWidth), (k_ScreenHeight / k_TileHeight));

    TickData data{game};
    data.renderer = renderer;
    data.font = TTF_OpenFont("resources/RobotoMono-Regular.ttf", 30/*size*/);
    AbortOnSDLError(data.font);

    MainLoop(data);

    return 0;
}
