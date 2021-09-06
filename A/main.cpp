#include <bits/stdc++.h>

template<class T> inline bool chmax(T& a, T b) { if (a < b) { a = b; return 1; } return 0; }
template<class T> inline bool chmin(T& a, T b) { if (a > b) { a = b; return 1; } return 0; }
const int dr[4] = {1, 0, -1, 0};
const int dc[4] = {0, 1, 0, -1};

using std::cout;
using std::endl;
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
std::vector<std::pair<int, int>> road;
std::vector<Vegetable> untreated;
struct Game {
    std::vector<std::vector<int>> has_machine;
    std::vector<std::vector<int>> vege_values;
    std::vector<std::vector<int>> done;
    std::vector<std::vector<int>> use_load;
    std::vector<std::vector<int>> value;
    int num_machine;
    int next_price;
    int money;

    Game() : num_machine(0), next_price(1), money(1)
    {
        has_machine.assign(N, std::vector<int>(N, 0));
        vege_values.assign(N, std::vector<int>(N, 0));
        done.assign(N, std::vector<int>(N, 0));
        value.assign(N, std::vector<int>(N, 0));
        use_load.assign(N, std::vector<int>(N, 0));
    }

    void purchase(int r, int c)
    {
        assert(!has_machine[r][c]);
        assert(next_price <= money);
        has_machine[r][c] = 1;
        money -= next_price;
        num_machine++;
        next_price = (num_machine + 1) * (num_machine + 1) * (num_machine + 1);
    }

    void move(int r1, int c1, int r2, int c2)
    {
        assert(has_machine[r1][c1]);
        has_machine[r1][c1] = 0;
        assert(!has_machine[r2][c2]);
        has_machine[r2][c2] = 1;
    }

    void appear(const int day)
    {
      // appear
      for (const Vegetable& vege : veges_start[day])
      {
          untreated.emplace_back(vege);
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
        for (int i = 0; i < N; i++)
        {
            for (int j = 0; j < N; j++)
            {
                if (has_machine[i][j] and vege_values[i][j] > 0)
                {
                    money += vege_values[i][j] * count_connected_machines(i, j);
                    vege_values[i][j] = 0;
                }
            }
        }
        // disappear
        for (const Vegetable& vege : veges_end[day])
        {
            vege_values[vege.r][vege.c] = 0;
        }
    }

    int count_connected_machines(int r, int c)
    {
        std::vector<std::pair<int, int>> queue = {{r, c}};
        std::vector<std::vector<int>> visited(N, std::vector<int>(N, 0));
        visited[r][c] = 1;
        int i = 0;
        while (i < (int)queue.size()) {
            int cr = queue[i].first;
            int cc = queue[i].second;
            for (int dir = 0; dir < 4; dir++)
            {
                int nr = cr + dr[dir];
                int nc = cc + dc[dir];
                if (0 <= nr && nr < N && 0 <= nc && nc < N && has_machine[nr][nc] && !visited[nr][nc])
                {
                    visited[nr][nc] = 1;
                    queue.push_back({nr, nc});
                }
            }
            i++;
        }
        return i;
    }

    Action select_next_action(int day)
    {
      bool ispurchase = (day < 500 and money >= next_price);
      if(road.empty())
      {
        std::sort(untreated.rbegin(), untreated.rend());
        while(not untreated.empty() and vege_values[untreated.back().r][untreated.back().c] == 0)
          untreated.pop_back();
        if(untreated.empty())
        {
          return Action::pass();
        }
        const int cnt = std::min({machine_count + ispurchase, 10, T - day});
        for (int i = std::max(0, day - 10); i < day + cnt; ++i)
        {
          for (const auto &vege : veges_start[i])
          {
            if(vege.e >= day + cnt - 1 and ((i < day and vege_values[vege.r][vege.c] > 0) or i >= day))
            {
              value[vege.r][vege.c] = vege.v;
            }
          }
        }
        {
          use_load.assign(N, std::vector<int>(N));
          std::vector<std::pair<int, int>> vp;
          int max = 0;
          road.clear();
          auto dfs = [&](auto &&self, int r, int c, int val, std::vector<std::pair<int, int>> &vp)
          {
            done[r][c] = true;
            vp.emplace_back(r, c);
            val += value[r][c];
            if((int)vp.size() == cnt)
            {
              if(chmax(max, val))
              {
                road = vp;
              }
              done[r][c] = false;
              vp.pop_back();
              val -= value[r][c];
              return;
            }
            for (int i = 0; i < 4; ++i)
            {
              const int nr = r + dr[i];
              const int nc = c + dc[i];
              if(nr < 0 or nr >= N or nc < 0 or nc >= N)
                continue;
              if(done[nr][nc])
                continue;
              self(self, nr, nc, val, vp);
            }
            done[r][c] = false;
            vp.pop_back();
            val -= value[r][c];
          };

          for (int i = 0; i < N; ++i)
          {
            for (int j = 0; j < N; ++j)
            {
              dfs(dfs, i, j, 0, vp);
            }
          }
          for (const auto &[r, c] : road)
          {
            use_load[r][c] = true;
          }
        }
        std::sort(road.begin(), road.end(), [&](auto i, auto j)
        {
          return value[i.first][i.second] > value[j.first][j.second];
        });
        for (int i = std::max(0, day - 10); i < day + cnt; ++i)
        {
          for (const auto &vege : veges_start[i])
          {
            value[vege.r][vege.c] = 0;
          }
        }
      }
      if(not road.empty())
      {
        const auto [r, c] = road.back();
        road.pop_back();
        if(has_machine[r][c])
        {
          return Action::pass();
        }
        if(ispurchase)
        {
          machine_count += 1;
          return Action::purchase(r, c);
        }
        for (int i = 0; i < N; ++i)
        {
          for (int j = 0; j < N; ++j)
          {
            if(use_load[i][j])
              continue;
            if(has_machine[i][j])
            {
              return Action::move(i, j, r, c);
            }
          }
        };
        return Action::pass();
      }
      return Action::pass();
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
