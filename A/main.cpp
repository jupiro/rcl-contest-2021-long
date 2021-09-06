#include <bits/stdc++.h>

template<class T> inline bool chmax(T& a, T b) { if (a < b) { a = b; return 1; } return 0; }
template<class T> inline bool chmin(T& a, T b) { if (a > b) { a = b; return 1; } return 0; }
const int dr[] = {1, 0, -1, 0};
const int dc[] = {0, 1, 0, -1};

const int inf = (int)1e9 + 7;
using ll = long long;
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
std::vector<Vegetable> untreated;
struct Game {
    std::vector<std::vector<int>> has_machine;
    std::vector<std::vector<int>> vege_values;
    std::vector<std::vector<int>> deadline;
    std::vector<std::vector<int>> dist;
    std::vector<std::vector<int>> back;
    std::vector<std::pair<int, int>> road;
    std::vector<std::pair<int, int>> pos;
    std::vector<std::vector<std::tuple<int, int, int>>> connected_values;
    std::pair<int, int> destination;
    int num_machine;
    int next_price;
    int money;

    Game() : num_machine(0), next_price(1), money(1)
    {
        has_machine.assign(N, std::vector<int>(N, 0));
        vege_values.assign(N, std::vector<int>(N, 0));
        deadline.assign(N, std::vector<int>(N));
        dist.assign(N, std::vector<int>(N));
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
      if(untreated.empty())
        return Action::pass();
      const bool ispurchace = (day < 800 and money >= next_price);
      if((machine_count + ispurchace) >= 2 and road.empty())
      {
        std::queue<std::pair<int, int>> q;
        dist.assign(N, std::vector<int>(N, inf));
        back.assign(N, std::vector<int>(N, -1));
        pos.clear();
        for (int i = 0; i < N; ++i)
        {
          for (int j = 0; j < N; ++j)
          {
            if(has_machine[i][j])
            {
              pos.emplace_back(i, j);
              dist[i][j] = 0;
              q.emplace(i, j);
            }
          }
        }
        int max = 0;
        int d_min = inf;
        const int len_max = std::max(5, machine_count);
        while(not q.empty())
        {
          const auto [r, c] = q.front();
          q.pop();
          if(vege_values[r][c] > 0 and deadline[r][c] >= day + dist[r][c])
          {
            if(chmax(max, vege_values[r][c]))
            {
              destination = {r, c};
              d_min = dist[r][c];
            }
            else if(max == vege_values[r][c] and chmin(d_min, dist[r][c]))
            {
              destination = {r, c};
            }
          }
          if(dist[r][c] >= len_max)
            continue;
          for (int i = 0; i < 4; ++i)
          {
            const int nr = r + dr[i];
            const int nc = c + dc[i];
            if(nr < 0 or nr >= N or nc < 0 or nc >= N)
              continue;
            if(chmin(dist[nr][nc], dist[r][c] + 1))
            {
              back[nr][nc] = i;
              q.emplace(nr, nc);
            }
          }
        }
        if(destination.first == -1)
          return Action::pass();
        road.clear();
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
            int fi = -1, fj = -1;
            int max = -1;
            for (int i = 0; i < N; ++i)
            {
              for (int j = 0; j < N; ++j)
              {
                if(has_machine[i][j])
                {
                  for (int k = 0; k < 4; ++k)
                  {
                    const int nr = i + dr[k];
                    const int nc = j + dc[k];
                    if(nr < 0 or nr >= N or nc < 0 or nc >= N)
                      continue;
                    if(not has_machine[nr][nc] and chmax(max, vege_values[nr][nc]))
                    {
                      fi = nr, fj = nc;
                    }
                  }
                }
              }
            }
            return Action::purchase(fi, fj);
          }
          else
          {
            pos.emplace_back(road.back());
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
        int min = inf;
        int fr = -1, fc = -1;
        for (const auto &[r, c] : pos)
        {
          const auto &[tr, tc] = road.back();
          has_machine[tr][tc] = true;
          has_machine[r][c] = false;
          std::vector<std::vector<bool>> done(N, std::vector<bool>(N));
          std::queue<std::pair<int, int>> q;
          q.emplace(tr, tc);
          done[tr][tc] = true;
          int cnt = 1;
          while(not q.empty())
          {
            const auto [r, c] = q.front();
            q.pop();
            for (int i = 0; i < 4; ++i)
            {
              const int nr = r + dr[i];
              const int nc = c + dc[i];
              if(nr < 0 or nr >= N or nc < 0 or nc >= N)
                continue;
              if(has_machine[nr][nc] and not done[nr][nc])
              {
                done[nr][nc] = true;
                q.emplace(nr, nc);
                cnt += 1;
              }
            }
          }
          has_machine[tr][tc] = false;
          has_machine[r][c] = true;
          if(cnt == machine_count and chmin(min, vege_values[r][c]))
          {
            fr = r, fc = c;
          }
        }
        if(fr == -1)
          assert(false);
        const auto ret = Action::move(fr, fc, road.back().first, road.back().second);
        pos.erase(std::find(pos.begin(), pos.end(), std::make_pair(fr, fc)));
        pos.emplace_back(road.back());
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
