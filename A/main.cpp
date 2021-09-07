#include <bits/stdc++.h>

template<class T> inline bool chmax(T& a, T b) { if (a < b) { a = b; return 1; } return 0; }
template<class T> inline bool chmin(T& a, T b) { if (a > b) { a = b; return 1; } return 0; }
const int dr[] = {1, 0, -1, 0};
const int dc[] = {0, 1, 0, -1};

const int inf = (int)1e9 + 7;
using ll = long long;
using std::cout;
using std::endl;

uint32_t xor64(void)
{
  static uint64_t x = 88172645463325252ULL;
  x = x ^ (x << 13); x = x ^ (x >> 7);
  return x = x ^ (x << 17);
}

struct Vegetable
{
    int r, c, s, e, v;
    Vegetable() : r(-1), c(-1), s(-1), e(-1), v(-1) {}
    Vegetable(int r_, int c_, int s_, int e_, int v_)
     : r(r_), c(c_), s(s_), e(e_), v(v_) {}
     bool operator==(const Vegetable &K) const
     {
      return r == K.r and c == K.c and s == K.s;
     }
     bool operator<(const Vegetable &K) const
     {
      if(v == K.v)
        return e < K.e;
      return v > K.v;
     }
};

struct Action
{
    std::vector<int> vs;

private:
    explicit Action(const std::vector<int>& vs_) : vs(vs_) {}

public:
    static Action pass()
    {
        return Action({-1});
    }

    static Action purchase(int r, int c)
    {
        return Action({r, c});
    }

    static Action move(int r1, int c1, int r2, int c2)
    {
        return Action({r1, c1, r2, c2});
    }
};
struct Destination
{
  int r, c;
  double score;
  Destination(int r, int c, double score) : r(r), c(c), score(score) {}
  bool operator<(const Destination &d) const
  {
    return score < d.score;
  }
  bool operator>(const Destination &d) const
  {
    return score > d.score;
  }
};
int N = 16, M, T;
std::vector<std::vector<Vegetable>> veges_start; // veges_start[i] : vegetables appear on day i
std::vector<std::vector<Vegetable>> veges_end;   // veges_end[i] : vegetables disappear on day i


struct Common
{
  std::vector<std::vector<int>> dist;
  std::vector<std::vector<int>> back;
  std::vector<std::pair<int, int>> road;
  std::vector<std::vector<int>> ord, low;
  std::pair<int, int> destination;
  std::vector<std::pair<int, int>> not_articulation;
  std::queue<std::pair<int, int>> q;
  std::priority_queue<Destination, std::vector<Destination>, std::greater<>> destination_pq;
  int beam_width;
  Common ()
  {
    beam_width = 7;
    dist.assign(N, std::vector<int>(N));
    ord.assign(N, std::vector<int>(N, inf));
    low.assign(N, std::vector<int>(N, inf));
    destination = {-1, -1};
  }
};
Common common;

struct KKT89
{
  std::array<std::array<bool, 16>, 16> has_machine;
  std::array<std::array<int, 16>, 16> vege_values, deadline;
  std::vector<std::pair<int, int>> pos;
  std::vector<Action> actions;
  int num_machine;
  int next_price;
  int money;
  KKT89() : num_machine(0), next_price(1), money(1)
  {
    const int N = 16;
    for (int i = 0; i < N; ++i)
    {
      for (int j = 0; j < N; ++j)
      {
        has_machine[i][j] = false;
        vege_values[i][j] = deadline[i][j] = 0;
      }
    }
  }
  bool operator<(const KKT89 &kkt89) const
  {
    if(num_machine == kkt89.num_machine)
    {
      return money < kkt89.money;
    }
    else
      return num_machine < kkt89.num_machine;
  }
  bool operator>(const KKT89 &kkt89) const
  {
    if(num_machine == kkt89.num_machine)
    {
      return money > kkt89.money;
    }
    else
      return num_machine > kkt89.num_machine;
  }
};


struct Game
{
    static void purchase(int r, int c, KKT89 &state)
    {
        assert(!state.has_machine[r][c]);
        assert(state.next_price <= state.money);
        state.has_machine[r][c] = 1;
        state.money -= state.next_price;
        state.num_machine++;
        state.next_price = (state.num_machine + 1) * (state.num_machine + 1) * (state.num_machine + 1);

        state.pos.emplace_back(r, c);
    }

    static void move(int r1, int c1, int r2, int c2, KKT89 &state)
    {
        assert(state.has_machine[r1][c1]);
        state.has_machine[r1][c1] = 0;
        assert(!state.has_machine[r2][c2]);
        state.has_machine[r2][c2] = 1;

        auto itr = std::find(state.pos.begin(), state.pos.end(), std::make_pair(r1, c1));
        *itr = {r2, c2};
    }

    static void appear(const int day, KKT89 &state)
    {
      // appear
      for (const Vegetable& vege : veges_start[day])
      {
          state.deadline[vege.r][vege.c] = vege.e;
          state.vege_values[vege.r][vege.c] = vege.v;
      }
    }
    static void simulate(int day, const Action& action, KKT89 &state)
    {
        // apply
        if (action.vs.size() == 2)
        {
            purchase(action.vs[0], action.vs[1], state);
        }
        else if (action.vs.size() == 4)
        {
            move(action.vs[0], action.vs[1], action.vs[2], action.vs[3], state);
        }

        // harvest
        for (const auto &[r, c] : state.pos)
        {
          if(state.vege_values[r][c] > 0)
          {
            state.money += state.vege_values[r][c] * (int)state.pos.size();
            state.vege_values[r][c] = 0;
          }
        }

        // disappear
        for (const Vegetable& vege : veges_end[day])
        {
            state.vege_values[vege.r][vege.c] = 0;
        }
    }

    static void bfs(const int len_max, const KKT89 &state)
    {
      common.dist.assign(N, std::vector<int>(N, inf));
      common.back.assign(N, std::vector<int>(N, -1));
      for (const auto &[r, c] : state.pos)
      {
        common.q.emplace(r, c);
        common.dist[r][c] = 0;
      }
      while(not common.q.empty())
      {
        const auto [r, c] = common.q.front();
        common.q.pop();
        if(common.dist[r][c] >= len_max)
          continue;
        for (int i = 0; i < 4; ++i)
        {
          const int nr = r + dr[i];
          const int nc = c + dc[i];
          if (nr < 0 or nr >= N or nc < 0 or nc >= N)
            continue;
          if(chmin(common.dist[nr][nc], common.dist[r][c] + 1))
          {
            common.back[nr][nc] = i;
            common.q.emplace(nr, nc);
          }
          else if(common.dist[nr][nc] == common.dist[r][c] + 1 and state.vege_values[r][c] > 0)
          {
            common.back[nr][nc] = i;
          }
        }
      }
    }

    static void calc_destination(const int day, const KKT89 &state)
    {
      common.destination = {-1, -1};
      for (int r = 0; r < N; ++r)
      {
        for (int c = 0; c < N; ++c)
        {
          if(state.vege_values[r][c] > 0 and state.deadline[r][c] >= day + common.dist[r][c] - 1)
          {
            if(common.dist[r][c] > 0)
            {
              common.destination_pq.emplace(r, c, (double)state.vege_values[r][c] / common.dist[r][c]);
              while ((int)common.destination_pq.size() > common.beam_width)
                common.destination_pq.pop();
            }
          }
        }
      }
    }
    static void construct_road()
    {
      int cr = common.destination.first, cc = common.destination.second;
      while(common.dist[cr][cc] > 0)
      {
        common.road.emplace_back(cr, cc);
        const int ddr = dr[common.back[cr][cc]];
        const int ddc = dc[common.back[cr][cc]];
        cr -= ddr;
        cc -= ddc;
      }
    }

    static void search_not_articulation(const int fr, const int fc, KKT89 &state)
    {
      common.not_articulation.clear();
      int idx = 0;
      auto dfs = [&](auto &&self, int r, int c, int pr, int pc)->void
      {
        common.ord[r][c] = common.low[r][c] = idx;
        idx += 1;
        bool is_articulation = false;
        for (int i = 0; i < 4; ++i)
        {
          const int nr = r + dr[i];
          const int nc = c + dc[i];
          if(nr < 0 or nr >= N or nc < 0 or nc >= N)
            continue;
          if(nr == pr and nc == pc)
            continue;
          if(not (state.has_machine[nr][nc] or (nr == fr and nc == fc)))
            continue;
          if(common.ord[nr][nc] == inf)
          {
            self(self, nr, nc, r, c);
            if(pr >= 0 and common.low[nr][nc] >= common.ord[r][c])
            {
              is_articulation = true;
            }
            chmin(common.low[r][c], common.low[nr][nc]);
          }
          else
          {
            chmin(common.low[r][c], common.ord[nr][nc]);
          }
        }
        if((not is_articulation) and pr >= 0)
          common.not_articulation.emplace_back(r, c);
      };
      dfs(dfs, fr, fc, -1, -1);
      common.ord[fr][fc] = common.low[fr][fc] = inf;
      for (const auto &[r, c] : state.pos)
      {
        common.ord[r][c] = common.low[r][c] = inf;
      }
      int min_vege_values = inf;
      for (const auto &[r, c] : common.not_articulation)
      {
        chmin(min_vege_values, state.vege_values[r][c]);
      }
      for (auto itr = common.not_articulation.begin(); itr != common.not_articulation.end();)
      {
        const auto &[r, c] = *itr;
        if(min_vege_values == state.vege_values[r][c])
          itr++;
        else
          itr = common.not_articulation.erase(itr);
      }
    }

    static void start_construct(int day, const KKT89 &state)
    {
      const bool ispurchace = (day < 840 and state.money >= state.next_price);
      if((state.num_machine + ispurchace) >= 2)
      {
        const int len_max = 6;
        bfs(len_max, state);
        calc_destination(day, state);
      }
    }
    static Action select_next_action(int day, KKT89 &state)
    {
      const bool ispurchace = (day < 840 and state.money >= state.next_price);
      if(ispurchace)
      {
        if(state.num_machine == 0)
        {
          int fi = 0, fj = 0;
          int max = 0;
          for (int i = 0; i < N; ++i)
          {
            for (int j = 0; j < N; ++j)
            {
              if(chmax(max, state.vege_values[i][j]))
                fi = i, fj = j;
            }
          }
          return Action::purchase(fi, fj);
        }
        else
        {
          if(common.road.empty())
          {
            return Action::pass();
          }
          else
          {
            const auto ret = Action::purchase(common.road.back().first, common.road.back().second);
            common.road.pop_back();
            return ret;
          }
        }
      }
      else
      {
        if(state.num_machine == 1)
        {
          int max = -1;
          int fr = -1, fc = -1;
          int sr = -1, sc = -1;
          for (int i = 0; i < N; ++i)
          {
            for (int j = 0; j < N; ++j)
            {
              if(state.has_machine[i][j])
              {
                sr = i, sc = j;
              }
              if(chmax(max, state.vege_values[i][j]))
              {
                fr = i, fc = j;
              }
            }
          }
          return Action::move(sr, sc, fr, fc);
        }
        if(common.road.empty())
        {
          return Action::pass();
        }
        search_not_articulation(common.road.back().first, common.road.back().second, state);
        if(common.not_articulation.empty())
        {
          return Action::pass();
        }
        const int idx = xor64() % (int)common.not_articulation.size();
        const auto &[fr, fc] = common.not_articulation[idx];
        const auto ret = Action::move(fr, fc, common.road.back().first, common.road.back().second);
        common.road.pop_back();
        return ret;
      }
    }
};

int main()
{
    std::cin >> N >> M >> T;
    veges_start.resize(T);
    veges_end.resize(T);
    for (int i = 0; i < M; i++)
    {
        int r, c, s, e, v;
        std::cin >> r >> c >> s >> e >> v;
        Vegetable vege(r, c, s, e, v);
        veges_start[s].push_back(vege);
        veges_end[e].push_back(vege);
    }
    KKT89 state, n_state;
    std::vector<std::priority_queue<KKT89, std::vector<KKT89>, std::greater<>>> beam(T + 1);
    beam[0].emplace(state);
    for (int day = 0; day < T; day++)
    {
      auto &pq = beam[day];
      while(not pq.empty())
      {
        state = pq.top();
        pq.pop();
        Game::appear(day, state);
        Game::start_construct(day, state);
        if(common.destination_pq.empty())
        {
          Action action = Game::select_next_action(day, state);
          state.actions.emplace_back(action);
          Game::simulate(day, action, state);
          beam[day + 1].emplace(state);
          if((int)beam[day + 1].size() > common.beam_width)
          {
            beam[day + 1].pop();
          }
        }
        while(not common.destination_pq.empty())
        {
          common.destination.first = common.destination_pq.top().r;
          common.destination.second = common.destination_pq.top().c;
          common.destination_pq.pop();
          Game::construct_road();
          int cday = day;
          n_state = state;
          Action action = Game::select_next_action(cday, n_state);
          n_state.actions.emplace_back(action); 
          Game::simulate(cday, action, n_state);
          while(not common.road.empty() and cday < T)
          {
            cday += 1;
            Game::appear(cday, n_state);
            action = Game::select_next_action(cday, n_state);
            n_state.actions.emplace_back(action);
            Game::simulate(cday, action, n_state);
          }
          beam[cday + 1].push(n_state);
          if((int)beam[cday + 1].size() > common.beam_width)
          {
            beam[cday + 1].pop();
          }
        }
      }
    }
    while ((int)beam[T].size() > 1)
      beam[T].pop();
    for (const Action& action : beam[T].top().actions)
    {
        for (int i = 0; i < (int)action.vs.size(); i++) {
            std::cout << action.vs[i] << (i == (int)action.vs.size() - 1 ? "\n" : " ");
        }
    }
}
