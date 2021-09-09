
#pragma GCC optimize("O3")
#pragma GCC optimize("unroll-loops")
#include <bits/stdc++.h>

template<class T> inline bool chmax(T& a, T b) { if (a < b) { a = b; return 1; } return 0; }
template<class T> inline bool chmin(T& a, T b) { if (a > b) { a = b; return 1; } return 0; }
const int dr[] = {1, 0, -1, 0};
const int dc[] = {0, 1, 0, -1};

const int inf = (int)1000;
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
  static int pass()
  {
    return -1;
  }

  static int purchase(int r, int c)
  {
    const int N = 16;
    return r * N + c;
  }

  static int move(int r1, int c1, int r2, int c2)
  {
    const int N = 16;
    return r1 * N * N * N + c1 * N * N + r2 * N + c2 + N * N;
  }

  static std::pair<int, int> dec_purchase(const int action)
  {
    const int N = 16;
    return std::pair<int, int>(action / N, action % N);
  }

  static std::tuple<int, int, int, int> dec_move(int action)
  {
    const int N = 16;
    action -= N * N;
    const int c2 = action % N;
    action -= c2;
    action /= N;
    const int r2 = action % N;
    action -= r2;
    action /= N;
    const int c1 = action % N;
    action -= c1;
    action /= N;
    const int r1 = action;
    return std::make_tuple(r1, c1, r2, c2);
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
const int N = 16, M = 5000, T = 1000;
std::vector<std::vector<Vegetable>> veges_start; // veges_start[i] : vegetables appear on day i
std::vector<std::vector<Vegetable>> veges_end;   // veges_end[i] : vegetables disappear on day i


struct Common
{
  std::array<std::array<bool, 16>, 16> has_machine;
  std::array<std::array<std::array<int, 16>, 16>, 1000> deadline_table;
  std::array<std::array<short, 16>, 16> dist, back, ord, low;
  std::vector<std::pair<int, int>> road;
  std::pair<int, int> destination;
  std::vector<std::pair<int, int>> not_articulation;
  std::queue<std::pair<int, int>> q;
  std::priority_queue<Destination, std::vector<Destination>, std::greater<>> destination_pq;
  int beam_width;
  int destination_width;
  Common ()
  {
    beam_width = 7;
    destination_width = 3;
    for (int i = 0; i < N; ++i)
    {
      for (int j = 0; j < N; ++j)
      {
        dist[i][j] = ord[i][j] = low[i][j] = inf;
      }
    }
    for (int i = 0; i < N; ++i)
    {
      for (int j = 0; j < N; ++j)
      {
        has_machine[i][j] = 0;
      }
    }

    destination = {-1, -1};
  }
};
Common common;

struct KKT89
{
  
  std::array<std::array<int, 16>, 16> vege_values;
  std::vector<std::pair<int, int>> pos;
  std::vector<int> actions;
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
        vege_values[i][j] = 0;
      }
    }
  }
  bool operator<(const KKT89 &kkt89) const
  {
    const int val = num_machine * (num_machine + 1) / 2;
    const int kval = kkt89.num_machine * (kkt89.num_machine + 1) / 2;
    return val * val + money < kval * kval + kkt89.money;
  }
  bool operator>(const KKT89 &kkt89) const
  {
    const int val = num_machine * (num_machine + 1) / 2;
    const int kval = kkt89.num_machine * (kkt89.num_machine + 1) / 2;
    return val * val + money > kval * kval + kkt89.money;
  }
};

int calc_score(const KKT89 &kkt89)
{
  const int kval = kkt89.num_machine * (kkt89.num_machine + 1) / 2;
  return kval * kval + kkt89.money;
}

struct Game
{
    static void has_machine_in(const KKT89 &state)
    {
      for (const auto &[r, c] : state.pos)
        common.has_machine[r][c] = true;
    }
    static void has_machine_out(const KKT89 &state)
    {
      for (const auto &[r, c] : state.pos)
        common.has_machine[r][c] = false;
    }
    static void purchase(int r, int c, KKT89 &state)
    {
        assert(!common.has_machine[r][c]);
        assert(state.next_price <= state.money);
        common.has_machine[r][c] = 1;
        state.money -= state.next_price;
        state.num_machine++;
        state.next_price = (state.num_machine + 1) * (state.num_machine + 1) * (state.num_machine + 1);

        state.pos.emplace_back(r, c);
    }

    static void move(int r1, int c1, int r2, int c2, KKT89 &state)
    {
        assert(common.has_machine[r1][c1]);
        common.has_machine[r1][c1] = 0;
        assert(!common.has_machine[r2][c2]);
        common.has_machine[r2][c2] = 1;

        auto itr = std::find(state.pos.begin(), state.pos.end(), std::make_pair(r1, c1));
        *itr = {r2, c2};
    }

    static void appear(const int day, KKT89 &state)
    {
      // appear
      for (const Vegetable& vege : veges_start[day])
      {
          state.vege_values[vege.r][vege.c] = vege.v;
      }
    }
    static void simulate(int day, const int action, KKT89 &state)
    {
        // apply
        if (0 <= action and action < N * N)
        {
          const auto &[r, c] = Action::dec_purchase(action);
          purchase(r, c, state);
        }
        else if (action >= N * N)
        {
          const auto &[r1, c1, r2, c2] = Action::dec_move(action);
          move(r1, c1, r2, c2, state);
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
      for (int i = 0; i < N; ++i)
      {
        for (int j = 0; j < N; ++j)
        {
          common.dist[i][j] = inf;
        }
      }
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
          if(chmin<short>(common.dist[nr][nc], common.dist[r][c] + 1))
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
      const auto &deadline = common.deadline_table[day];
      double max_vege_value = 0;
      for (int r = 0; r < N; ++r)
      {
        for (int c = 0; c < N; ++c)
        {
          if(state.vege_values[r][c] > 0 and deadline[r][c] >= day + common.dist[r][c] - 1)
          {
            if(common.dist[r][c] > 0 and chmax(max_vege_value, (double)state.vege_values[r][c] / common.dist[r][c]))
              common.destination = {r, c};
          }
        }
      }
      for (int i = day + 1; i < std::min((int)veges_start.size(), day + 6); ++i)
      {
        for (const auto &vege : veges_start[i])
        {
          if(common.dist[vege.r][vege.c] > 0)
          {
            if(vege.e >= day + common.dist[vege.r][vege.c] - 1 and vege.s <= day + common.dist[vege.r][vege.c] - 1)
            {
              if(chmax(max_vege_value, (double)vege.v / common.dist[vege.r][vege.c]))
                common.destination = {vege.r, vege.c};
            }
          }
        }
      }
    }
    static void calc_destination_pq(const int day, const KKT89 &state)
    {
      common.destination = {-1, -1};
      const auto &deadline = common.deadline_table[day];
      for (int r = 0; r < N; ++r)
      {
        for (int c = 0; c < N; ++c)
        {
          if(state.vege_values[r][c] > 0 and deadline[r][c] >= day + common.dist[r][c] - 1)
          {
            if(common.dist[r][c] > 0)
            {
              const double score = (double)state.vege_values[r][c];
              common.destination_pq.emplace(r, c, score);
              while ((int)common.destination_pq.size() > common.destination_width)
                common.destination_pq.pop();
            }
          }
        }
      }
      for (int i = day + 1; i < std::min((int)veges_start.size(), day + 6); ++i)
      {
        for (const auto &vege : veges_start[i])
        {
          if(common.dist[vege.r][vege.c] > 0)
          {
            if(vege.e >= day + common.dist[vege.r][vege.c] - 1 and vege.s <= day + common.dist[vege.r][vege.c] - 1)
            {
              common.destination_pq.emplace(vege.r, vege.c, vege.v);
              while ((int)common.destination_pq.size() > common.destination_width)
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
          if(not (common.has_machine[nr][nc] or (nr == fr and nc == fc)))
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

    static void start_construct(int day, bool ispq, const KKT89 &state)
    {
      const bool ispurchace = (state.num_machine < 50 and state.money >= state.next_price);
      if((state.num_machine + ispurchace) >= 2)
      {
        const int len_max = 6;
        bfs(len_max, state);
        if(ispq)
          calc_destination_pq(day, state);
        else
          calc_destination(day, state);
      }
    }
    static int select_next_action(KKT89 &state)
    {
      const bool ispurchace = (state.num_machine < 50 and state.money >= state.next_price);
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
              if(common.has_machine[i][j])
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
    int _N, _M, _T;  
    std::cin >> _N >> _M >> _T;
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
    for (int day = 0; day < T; ++day)
    {
      if(day == 0)
      {
        for (int i = 0; i < N; ++i)
        {
          for (int j = 0; j < N; ++j)
          {
            common.deadline_table[day][i][j] = 0;
          }
        }
      }
      else
      {
        common.deadline_table[day] = common.deadline_table[day - 1];
      }
      for (const Vegetable& vege : veges_start[day])
      {
          common.deadline_table[day][vege.r][vege.c] = vege.e;
      }
    }
    KKT89 state, n_state;
    std::vector<int> actions;
    std::vector<std::priority_queue<KKT89, std::vector<KKT89>, std::greater<>>> beam(T + 1);
    int day = 0;
    beam[day].emplace(state);
    for (; day < T; day++)
    {
      common.beam_width = 200;
      if(day < 600)
        common.beam_width = 80;
      if(day < 300)
        common.beam_width = 10;
      auto &pq = beam[day];
      while(not pq.empty())
      {
        if((int)pq.size() <= common.beam_width / 4)
        {
          common.destination_width = 5;
        }
        else
        {
          common.destination_width = 2;
        }
        state = pq.top();
        pq.pop();
        Game::appear(day, state);
        Game::start_construct(day, true, state);
        if(common.destination_pq.empty())
        {
          Game::has_machine_in(state);
          int action = Game::select_next_action(state);
          state.actions.emplace_back(action);
          Game::simulate(day, action, state);
          beam[day + 1].emplace(state);
          Game::has_machine_out(state);
          if((int)beam[day + 1].size() > common.beam_width)
          {
            beam[day + 1].pop();
          }
        }
        n_state = state;
        while(not common.destination_pq.empty())
        {
          Game::has_machine_in(state);
          common.destination.first = common.destination_pq.top().r;
          common.destination.second = common.destination_pq.top().c;
          common.destination_pq.pop();
          Game::construct_road();
          int cday = day;
          n_state = state;
          int action = Game::select_next_action(n_state);
          const int sz = n_state.actions.size();
          n_state.actions.reserve(sz + 1);
          n_state.actions.emplace_back(action); 
          Game::simulate(cday, action, n_state);
          while(not common.road.empty() and cday < T)
          {
            cday += 1;
            Game::appear(cday, n_state);
            action = Game::select_next_action(n_state);
            const int sz = n_state.actions.size();
            n_state.actions.reserve(sz + 1);
            n_state.actions.emplace_back(action);
            Game::simulate(cday, action, n_state);
          }
          Game::has_machine_out(n_state);
          {
            beam[cday + 1].emplace(n_state);
          }
          while((int)beam[cday + 1].size() > common.beam_width)
          {
            beam[cday + 1].pop();
          }
        }
      }
    }
    while ((int)beam[T].size() > 1)
      beam[T].pop();
    for (const int action : beam[T].top().actions)
      actions.emplace_back(action);
    for (const int& action : actions)
    {
      if(action < 0)
        cout << -1 << "\n";
      else if(action < N * N)
      {
        const auto &[r, c] = Action::dec_purchase(action);
        cout << r << " " << c << "\n";
      }
      else
      {
        const auto &[r1, c1, r2, c2] = Action::dec_move(action);
        cout << r1 << " " << c1 << " " << r2 << " " << c2 << "\n";
      }
    }
}