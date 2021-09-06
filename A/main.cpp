#include <bits/stdc++.h>

template<class T> inline bool chmax(T& a, T b) { if (a < b) { a = b; return 1; } return 0; }
template<class T> inline bool chmin(T& a, T b) { if (a > b) { a = b; return 1; } return 0; }
const int dr[] = {1, 0, -1, 0};
const int dc[] = {0, 1, 0, -1};

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
std::vector<std::pair<int, int>> road;
std::vector<Vegetable> untreated;
struct Game {
    std::vector<std::vector<int>> has_machine;
    std::vector<std::vector<int>> vege_values;
    std::vector<std::vector<int>> done;
    std::vector<std::vector<std::tuple<int, int, int>>> connected_values;
    int num_machine;
    int next_price;
    int money;

    Game() : num_machine(0), next_price(1), money(1)
    {
        has_machine.assign(N, std::vector<int>(N, 0));
        vege_values.assign(N, std::vector<int>(N, 0));
        done.assign(N, std::vector<int>(N));
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
      const bool ispurchace = (day < 500 and money >= next_price);
      {
        int id = 1;
        int val = 0;
        int cnt = 0;
        std::vector<std::pair<int, int>> vp;
        done.assign(N, std::vector<int>(N));
        connected_values.assign(N, std::vector<std::tuple<int, int, int>>(N));
        auto dfs = [&](auto &&self, int r, int c)->void
        {
          cnt += 1;
          val += vege_values[r][c];
          vp.emplace_back(r, c);
          done[r][c] = true;
          for (int i = 0; i < 4; ++i)
          {
            const int nr = r + dr[i];
            const int nc = c + dc[i];
            if(nr < 0 or nr >= N or nc < 0 or nc >= N)
              continue;
            if(done[nr][nc])
              continue;
            if(not has_machine[nr][nc])
              continue;
            self(self, nr, nc);
          }
        };
        for (int i = 0; i < N; ++i)
        {
          for (int j = 0; j < N; ++j)
          {
            if(done[i][j])
              continue;
            val = 0;
            cnt = 0;
            vp.clear();
            if(not has_machine[i][j])
              continue;
            dfs(dfs, i, j);
            for (const auto &[r, c] : vp)
            {
              connected_values[r][c] = std::make_tuple(val, cnt, id);
            }
            id += 1;
          }
        }
        if(ispurchace)
        {
          ll max = 0;
          int fr = -1, fc = -1;
          for (int r = 0; r < N; ++r)
          {
            for (int c = 0; c < N; ++c)
            {
              if(has_machine[r][c])
                continue;
              for (int k = 0; k < 4; ++k)
              {
                const int nr = r + dr[k];
                const int nc = c + dc[k];
                if(nr < 0 or nr >= N or nc < 0 or nc >= N)
                  continue;
                const auto &[val, cnt, id] = connected_values[nr][nc];
                const ll nxt = 1LL * (val + vege_values[r][c]) * (cnt + 1) * (cnt + 1);
                const ll cur = 1LL * val * cnt * cnt;
                if(chmax(max, nxt - cur))
                {
                  fr = r, fc = c;
                }
              }
            }
          }
          return Action::purchase(fr, fc);
        }
        else
        {
          int fr = -1, fc = -1, fi = -1, fj = -1;
          ll max = 0;

          for (int i = 0; i < N; ++i)
          {
            for (int j = 0; j < N; ++j)
            {
              if(not has_machine[i][j])
                continue;
              bool ok = true;
              int cnt = 0;
              for (int k = 0; k < 4; ++k)
              {
                const int nr = i + dr[k];
                const int nc = j + dc[k];
                if(nr < 0 or nr >= N or nc < 0 or nc >= N)
                  continue;
                if(has_machine[nr][nc])
                {
                  cnt += 1;
                  bool f = false;
                  for (int l = 0; l < 4; ++l)
                  {
                    const int nnr = nr + dr[l];
                    const int nnc = nc + dc[l];
                    if(nnr < 0 or nnr >= N or nnc < 0 or nnc >= N)
                      continue;
                    if(nnr == i and nnc == j)
                      continue;
                    if(has_machine[nnr][nnc])
                    {
                      f = true;
                      break;
                    }
                  }
                  if(not f)
                    ok = false;
                }
              }
              ok |= (cnt == 1 or cnt == 0);
              if(ok)
              {
                const auto &[cval, ccnt, cid] = connected_values[i][j];
                for (int r = 0; r < N; ++r)
                {
                  for (int c = 0; c < N; ++c)
                  {
                    if(has_machine[r][c])
                      continue;
                    for (int k = 0; k < 4; ++k)
                    {
                      const int nr = r + dr[k];
                      const int nc = c + dc[k];
                      if(nr < 0 or nr >= N or nc < 0 or nc >= N)
                        continue;
                      const auto &[val, cnt, id] = connected_values[nr][nc];
                      if(id == std::get<2>(connected_values[i][j]))
                      {
                        const ll nxt = 1LL * (val + vege_values[r][c] - vege_values[i][j]) * cnt;
                        const ll cur = 1LL * val * cnt;
                        if(chmax(max, nxt - cur))
                        {
                          fi = i, fj = j;
                          fr = r, fc = c;
                        }
                      }
                      else
                      {
                        const ll nxt = 1LL * (val + vege_values[r][c]) * (cnt + 1) + 1LL * (cval - vege_values[i][j]) * (ccnt - 1);
                        const ll cur = 1LL * val * cnt * cnt + 1LL * cval * cnt;
                        if(chmax(max, nxt - cur))
                        {
                          fi = i, fj = j;
                          fr = r, fc = c;
                        }
                      }
                    }
                  }
                }
              }
            }
          }
          if(fi == -1)
            return Action::pass();
          else
            return Action::move(fi, fj, fr, fc);
        }
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
