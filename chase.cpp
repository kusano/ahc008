#include <iostream>
#include <vector>
#include <string>
#include <utility>
using namespace std;

int xor64() {
    static uint64_t x = 88172645463345263ULL;
    x ^= x<<13;
    x ^= x>> 7;
    x ^= x<<17;
    return int(x&0x7fffffff);
}

const int S = 30;
const int T = 300;

const string dir_obst = "udlr";
const string dir_move = "UDLR";
const int dir_x[] = {-1, 1, 0, 0};
const int dir_y[] = {0, 0, -1, 1};


struct Field
{
    int N, M;
    vector<int> px, py, pt;
    vector<int> hx, hy;
    vector<int> hx_old, hy_old;

    // 0: 空き, 1: 通行不能
    vector<vector<int>> F;
    vector<vector<int>> P;
    vector<vector<int>> H;
    int turn;
    int turn_sub;
    vector<vector<int>> D;
    vector<pair<int, int>> Q;

    Field(vector<int> px, vector<int> py, vector<int> pt, vector<int> hx, vector<int> hy)
        : N(int(px.size()))
        , M(int(hx.size()))
        , px(px)
        , py(py)
        , pt(pt)
        , hx(hx)
        , hy(hy)
        , F(S, vector<int>(S))
        , P(S, vector<int>(S))
        , H(S, vector<int>(S))
        , turn(0)
        , turn_sub(0)
        , D(S, vector<int>(S))
    {
        hx_old.resize(M);
        hy_old.resize(M);

        for (int i=0; i<N; i++)
            P[py[i]][px[i]]++;
        for (int i=0; i<M; i++)
            H[hy[i]][hx[i]]++;
    }

    string get_random_pet_move() const
    {
        if (!(M<=turn_sub))
            throw "not pet turn";

        int p = turn_sub-M;
        int n;
        switch (pt[p])
        {
        case 0: n = 1;
        case 1: n = 2;
        case 2: n = 3;
        case 3: n = 2;
        case 4: n = 2;
        }

        string move;
        int x = px[p];
        int y = py[p];
        for (int j=0; j<n; j++)
        {
            vector<int> D;
            for (int d=0; d<4; d++)
            {
                int tx = x + dir_x[d];
                int ty = y + dir_y[d];
                if (0<=tx && tx<S && 0<=ty && ty<S && F[ty][tx]==0)
                    D.push_back(d);
            }
            int d = D[xor64()%(int)D.size()];
            x += dir_x[d];
            y += dir_y[d];
            move += dir_move[d];
        }
        return move;
    }

    //  人の可能な動きを返す
    string get_human_moves() const
    {
        if (!(turn_sub<M))
            throw "not human turn";

        int h = turn_sub;
        string moves = ".";
        for (int d=0; d<4; d++)
        {
            int x = hx[h]+dir_x[d];
            int y = hy[h]+dir_y[d];
            if (0<=x && x<S && 0<=y && y<S &&
                F[y][x]==0 && P[y][x]==0 && H[y][x]==0)
            {
                bool ok = true;
                for (int d2=0; d2<4 && ok; d2++)
                {
                    int x2 = x+dir_x[d2];
                    int y2 = y+dir_y[d2];
                    if (0<=x2 && x2<S && 0<=y2 && y2<S &&
                        P[y2][x2]>0)
                        ok = false;
                }
                if (ok)
                    moves += dir_obst[d];
            }
        }
        for (int d=0; d<4; d++)
        {
            int x = hx[h]+dir_x[d];
            int y = hy[h]+dir_y[d];
            if (0<=x && x<S && 0<=y && y<S &&
                F[y][x]==0)
                moves += dir_move[d];
        }

        return moves;
    }

    void move(string move)
    {
        if (turn_sub<M)
        {
            int h = turn_sub;
            hx_old[h] = -1;
            for (int d=0; d<4; d++)
                if (move[0]==dir_obst[d])
                {
                    int x = hx[h];
                    int y = hy[h];
                    x += dir_x[d];
                    y += dir_y[d];
                    F[y][x] = 1;
                }
            for (int d=0; d<4; d++)
                if (move[0]==dir_move[d])
                {
                    int x = hx[h];
                    int y = hy[h];
                    hx_old[h] = x;
                    hy_old[h] = y;
                    // 元いた場所も通行不可にはできないから後で
                    //H[y][x]--;
                    x += dir_x[d];
                    y += dir_y[d];
                    H[y][x]++;
                    hx[h] = x;
                    hy[h] = y;
                }
        }
        else
        {
            int p = turn_sub-M;
            for (char m: move)
                for (int d=0; d<4; d++)
                    if (m==dir_move[d])
                    {
                        int x = px[p];
                        int y = py[p];
                        P[y][x]--;
                        x += dir_x[d];
                        y += dir_y[d];
                        P[y][x]++;
                        px[p] = x;
                        py[p] = y;
                    }

        }
        turn_sub++;
        if (turn_sub>=N+M)
        {
            for (int i=0; i<M; i++)
                if (hx_old[i]>=0)
                    H[hy_old[i]][hx_old[i]]--;

            turn_sub = 0;
            turn++;
        }
    }

    // Dを(ox, oy)からの距離に更新
    void dist(int ox, int oy)
    {
        for (int y=0; y<S; y++)
            for (int x=0; x<S; x++)
                D[y][x] = 9999;

        Q.clear();
        Q.push_back({ox, oy});
        D[oy][ox] = 0;

        for (int q=0; q<(int)Q.size(); q++)
        {
            int x = Q[q].first;
            int y = Q[q].second;
            for (int d=0; d<4; d++)
            {
                int tx = x+dir_x[d];
                int ty = y+dir_y[d];
                if (0<=tx && tx<S && 0<=ty && ty<S &&
                    F[ty][tx]==0 && D[ty][tx]==9999)
                {
                    D[ty][tx] = D[y][x]+1;
                    Q.push_back({tx, ty});
                }
            }
        }
    }
};

int main()
{
    int N;
    cin>>N;
    vector<int> px(N), py(N), pt(N);
    for (int i=0; i<N; i++)
    {
        cin>>px[i]>>py[i]>>pt[i];
        px[i]--;
        py[i]--;
        pt[i]--;
    }
    int M;
    cin>>M;
    vector<int> hx(M), hy(M);
    for (int i=0; i<M; i++)
    {
        cin>>hx[i]>>hy[i];
        hx[i]--;
        hy[i]--;
    }

    Field field(px, py, pt, hx, hy);
    for (int t=0; t<T; t++)
    {
        string ans;
        for (int i=0; i<M; i++)
        {
            field.dist(field.hx[i], field.hy[i]);

            int target = -1;
            int targetd = 9999;
            for (int j=0; j<N; j++)
            {
                int d = field.D[field.py[j]][field.px[j]];
                if (d<targetd)
                {
                    target = j;
                    targetd = d;
                }
            }

            string m;
            if (target!=-1)
            {
                field.dist(field.px[target], field.py[target]);

                int dist = field.D[field.hy[i]][field.hx[i]];
                if (dist<3)
                {
                    // 離れる
                    for (int d=0; d<4; d++)
                    {
                        int tx = field.hx[i]+dir_x[d];
                        int ty = field.hy[i]+dir_y[d];
                        if (0<=tx && tx<S && 0<=ty && ty<S &&
                            field.D[ty][tx]==dist+1)
                        {
                            m = dir_move[d];
                        }
                    }
                }
                else if (dist==3 || dist==4)
                {
                    // 通行不可にする
                    for (int d=0; d<4; d++)
                    {
                        int tx = field.hx[i]+dir_x[d];
                        int ty = field.hy[i]+dir_y[d];
                        if (0<=tx && tx<S && 0<=ty && ty<S &&
                            field.D[ty][tx]==dist-1 &&
                            field.F[ty][tx]==0 &&
                            field.H[ty][tx]==0 &&
                            field.P[ty][tx]==0)
                        {
                            bool ok = true;
                            for (int d2=0; d2<4; d2++)
                            {
                                int tx2 = tx+dir_x[d2];
                                int ty2 = ty+dir_y[d2];
                                if (0<=tx2 && tx2<S && 0<=ty2 && ty2<S &&
                                    field.P[ty2][tx2]>0)
                                    ok = false;
                            }
                            // 極端に狭くなるなら不可
                            vector<vector<int>> oldD = field.D;
                            field.F[ty][tx] = 1;

                            field.dist(field.hx[i], field.hy[i]);
                            int c = 0;
                            for (int y=0; y<S; y++)
                                for (int x=0; x<S; x++)
                                    if (field.D[y][x]<9999)
                                        c++;
                            if (c<S*S/16)
                                ok = false;
                            field.D = oldD;
                            field.F[ty][tx] = 0;

                            if (ok)
                                m = dir_obst[d];
                        }
                    }
                }
                else // dist>4
                {
                    // 近づく
                    for (int d=0; d<4; d++)
                    {
                        int tx = field.hx[i]+dir_x[d];
                        int ty = field.hy[i]+dir_y[d];
                        if (0<=tx && tx<S && 0<=ty && ty<S &&
                            field.D[ty][tx]==dist-1)
                        {
                            m = dir_move[d];
                        }
                    }
                }
            }
            if (m=="")
                m = ".";
            field.move(m);
            ans += m;
        }
        cout<<ans<<endl;

        for (int i=0; i<N; i++)
        {
            string move;
            cin>>move;
            field.move(move);
        }
    }
}
