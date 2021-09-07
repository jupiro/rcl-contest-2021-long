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

int N, M, T;
std::vector<std::vector<Vegetable>> veges_start; // veges_start[i] : vegetables appear on day i
std::vector<std::vector<Vegetable>> veges_end;   // veges_end[i] : vegetables disappear on day i
int machine_count = 0;
std::vector<Vegetable> untreated;
struct Game
{
    std::vector<std::vector<int>> has_machine;
    std::vector<std::vector<int>> vege_values;
    std::vector<std::vector<int>> deadline;
    std::vector<std::vector<int>> dist;
    std::vector<std::vector<int>> back;
    std::vector<std::pair<int, int>> road;
    std::vector<std::pair<int, int>> pos;
    std::vector<std::vector<int>> ord, low;
    std::pair<int, int> destination;
    std::vector<std::pair<int, int>> not_articulation;
    std::queue<std::pair<int, int>> q;
    int num_machine;
    int next_price;
    int money;

    Game() : num_machine(0), next_price(1), money(1)
    {
        has_machine.assign(N, std::vector<int>(N, 0));
        vege_values.assign(N, std::vector<int>(N, 0));
        deadline.assign(N, std::vector<int>(N));
        dist.assign(N, std::vector<int>(N));
        ord.assign(N, std::vector<int>(N, inf));
        low.assign(N, std::vector<int>(N, inf));
        destination = {-1, -1};
    }

    void purchase(int r, int c)
    {
        assert(!has_machine[r][c]);
        assert(next_price <= money);
        has_machine[r][c] = 1;
        money -= next_price;
        num_machine++;
        next_price = (num_machine + 1) * (num_machine + 1) * (num_machine + 1);

        pos.emplace_back(r, c);
    }

    void move(int r1, int c1, int r2, int c2)
    {
        assert(has_machine[r1][c1]);
        has_machine[r1][c1] = 0;
        assert(!has_machine[r2][c2]);
        has_machine[r2][c2] = 1;

        auto itr = std::find(pos.begin(), pos.end(), std::make_pair(r1, c1));
        *itr = {r2, c2};
    }

    void appear(const int day)
    {
      // appear
      for (const Vegetable& vege : veges_start[day])
      {
          untreated.emplace_back(vege);
          deadline[vege.r][vege.c] = vege.e;
          vege_values[vege.r][vege.c] = vege.v;
      }
    }
    void simulate(int day, const Action& action)
    {
        // apply
        if (action.vs.size() == 2)
        {
            purchase(action.vs[0], action.vs[1]);
        }
        else if (action.vs.size() == 4)
        {
            move(action.vs[0], action.vs[1], action.vs[2], action.vs[3]);
        }

        // harvest
        for (const auto &[r, c] : pos)
        {
          if(vege_values[r][c] > 0)
          {
            money += vege_values[r][c] * (int)pos.size();
            vege_values[r][c] = 0;
          }
        }

        // disappear
        for (const Vegetable& vege : veges_end[day])
        {
            vege_values[vege.r][vege.c] = 0;
        }
    }

    void bfs(const int len_max)
    {
      dist.assign(N, std::vector<int>(N, inf));
      back.assign(N, std::vector<int>(N, -1));
      for (const auto &[r, c] : pos)
      {
        q.emplace(r, c);
        dist[r][c] = 0;
      }
      while(not q.empty())
      {
        const auto [r, c] = q.front();
        q.pop();
        if(dist[r][c] >= len_max)
          continue;
        for (int i = 0; i < 4; ++i)
        {
          const int nr = r + dr[i];
          const int nc = c + dc[i];
          if (nr < 0 or nr >= N or nc < 0 or nc >= N)
            continue;
          if(chmin(dist[nr][nc], dist[r][c] + 1))
          {
            back[nr][nc] = i;
            q.emplace(nr, nc);
          }
          else if(dist[nr][nc] == dist[r][c] + 1 and vege_values[r][c] > 0)
          {
            back[nr][nc] = i;
          }
        }
      }
    }

    void calc_destination(const int day, const int len)
    {
      destination = {-1, -1};
      double max_vege_value = 0;
      for (int r = 0; r < N; ++r)
      {
        for (int c = 0; c < N; ++c)
        {
          if(vege_values[r][c] > 0 and deadline[r][c] >= day + dist[r][c] - 1)
          {
            if(dist[r][c] > 0 and chmax(max_vege_value, (double)vege_values[r][c] / dist[r][c]))
              destination = {r, c};
          }
        }
      }
      for (int i = day + 1; i < std::min(day + len, (int)veges_start.size()); ++i)
      {
        for (const auto &vege : veges_start[i])
        {
          const auto &[r, c] = std::pair(vege.r, vege.c);
          if(vege.s >= day + dist[r][c] - 1 and vege.e <= day + dist[r][c] - 1)
          {
            if(dist[r][c] > 0 and chmax(max_vege_value, (double)vege.v / dist[r][c]))
              destination = {r, c};
          }
        }
      }
    }
    void construct_road()
    {
      int cr = destination.first, cc = destination.second;
      while(dist[cr][cc] > 0)
      {
        road.emplace_back(cr, cc);
        const int ddr = dr[back[cr][cc]];
        const int ddc = dc[back[cr][cc]];
        cr -= ddr;
        cc -= ddc;
      }
    }

    void search_not_articulation(const int fr, const int fc, std::vector<std::pair<int, int>> &pos, std::vector<std::pair<int, int>> &not_articulation)
    {
      not_articulation.clear();
      int idx = 0;
      auto dfs = [&](auto &&self, int r, int c, int pr, int pc)->void
      {
        ord[r][c] = low[r][c] = idx;
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
          if(not (has_machine[nr][nc] or (nr == fr and nc == fc)))
            continue;
          if(ord[nr][nc] == inf)
          {
            self(self, nr, nc, r, c);
            if(pr >= 0 and low[nr][nc] >= ord[r][c])
            {
              is_articulation = true;
            }
            chmin(low[r][c], low[nr][nc]);
          }
          else
          {
            chmin(low[r][c], ord[nr][nc]);
          }
        }
        if((not is_articulation) and pr >= 0)
          not_articulation.emplace_back(r, c);
      };
      dfs(dfs, fr, fc, -1, -1);
      ord[fr][fc] = low[fr][fc] = inf;
      for (const auto &[r, c] : pos)
      {
        ord[r][c] = low[r][c] = inf;
      }
      int min_vege_values = inf;
      for (const auto &[r, c] : not_articulation)
      {
        chmin(min_vege_values, vege_values[r][c]);
      }
      for (auto itr = not_articulation.begin(); itr != not_articulation.end();)
      {
        const auto &[r, c] = *itr;
        if(min_vege_values == vege_values[r][c])
          itr++;
        else
          itr = not_articulation.erase(itr);
      }
    }
    Action select_next_action(int day)
    {
      if(untreated.empty())
        return Action::pass();
      const bool ispurchace = (day < 840 and money >= next_price);
      if((machine_count + ispurchace) >= 2 and road.empty())
      {
        const int len_max = 6;
        bfs(len_max);
        calc_destination(day, len_max);
        if(destination.first == -1)
          return Action::pass();
        construct_road();
      }
      if(ispurchace)
      {
        machine_count += 1;
        if(machine_count == 1)
        {
          int fi = -1, fj = -1;
          int max = 0;
          for (int i = 0; i < N; ++i)
          {
            for (int j = 0; j < N; ++j)
            {
              if(chmax(max, vege_values[i][j]))
                fi = i, fj = j;
            }
          }
          return Action::purchase(fi, fj);
        }
        else
        {
          if(road.empty())
          {
            return Action::pass();
          }
          else
          {
            const auto ret = Action::purchase(road.back().first, road.back().second);
            road.pop_back();
            return ret;
          }
        }
      }
      else
      {
        if(machine_count == 1)
        {
          int max = -1;
          int fr = -1, fc = -1;
          int sr = -1, sc = -1;
          for (int i = 0; i < N; ++i)
          {
            for (int j = 0; j < N; ++j)
            {
              if(has_machine[i][j])
              {
                sr = i, sc = j;
              }
              if(chmax(max, vege_values[i][j]))
              {
                fr = i, fc = j;
              }
            }
          }
          return Action::move(sr, sc, fr, fc);
        }
        if(road.empty())
        {
          return Action::pass();
        }
        search_not_articulation(road.back().first, road.back().second, pos, not_articulation);
        if(not_articulation.empty())
        {
          return Action::pass();
        }
        const int idx = xor64() % (int)not_articulation.size();
        const auto &[fr, fc] = not_articulation[idx];
        const auto ret = Action::move(fr, fc, road.back().first, road.back().second);
        road.pop_back();
        return ret;
      }
    }
};

int main() {
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
    Game game;
    std::vector<Action> actions;
    for (int day = 0; day < T; day++)
    {
        game.appear(day);
        Action action = game.select_next_action(day);
        actions.push_back(action);
        game.simulate(day, action);
    }
    for (const Action& action : actions)
    {
        for (int i = 0; i < (int)action.vs.size(); i++) {
            std::cout << action.vs[i] << (i == (int)action.vs.size() - 1 ? "\n" : " ");
        }
    }
}
