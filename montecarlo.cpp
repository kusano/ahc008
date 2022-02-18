#include <iostream>
#include <vector>
#include <string>
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

    // 0: 空き, 1: 通行不能
    vector<vector<int>> F;
    vector<vector<int>> P;
    vector<vector<int>> H;
    int turn;

    Field(vector<int> px, vector<int> py, vector<int> pt, vector<int> hx, vector<int> hy)
        : N(int(px.size()))
        , M(int(py.size()))
        , px(px)
        , py(py)
        , pt(pt)
        , hx(hx)
        , hy(hy)
        , F(S, vector<int>(S))
        , P(S, vector<int>(S))
        , H(S, vector<int>(S))
        , turn(0)
    {
        for (int i=0; i<N; i++)
            P[py[i]][px[i]]++;
        for (int i=0; i<M; i++)
            H[hy[i]][hx[i]]++;
    }

    vector<string> get_random_pet_moves() const
    {
        vector<string> ret(N);

        for (int i=0; i<N; i++)
        {
            int n;
            switch (pt[i])
            {
            case 0: n = 1;
            case 1: n = 2;
            case 2: n = 3;
            case 3: n = 2;
            case 4: n = 2;
            }

            string m;
            int x = px[i];
            int y = py[i];
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
                m += dir_move[d];
            }
            ret[i] = m;
        }
        return ret;
    }

    //  各人の可能な動きを返す
    vector<string> get_human_moves() const
    {
        vector<string> moves;
        for (int i=0; i<M; i++)
        {
            string m = ".";
            for (int d=0; d<4; d++)
            {
                int x = hx[i]+dir_x[d];
                int y = hy[i]+dir_y[d];
                if (0<=x && x<S && 0<=y && y<S &&
                    F[y][x]==0 && P[y][x]==0 && H[y][x]==0)
                {
                    bool ok = true;
                    for (int d2=0; d2<4 && ok; d2++)
                    {
                        int x2 = x+dir_x[d2];
                        int y2 = y+dir_y[d2];
                        if (0<=x2 && x2<S && 0<=y2 && y2<S &&
                            // とりあえず人に隣接しているところも不可
                            (P[y2][x2]>0 || (H[y2][x2]>0 && (x2!=hx[i]||y2!=hy[i])) || (H[y2][x2]>1 && (x2==hx[i]&&y2==hy[i])) ))
                            ok = false;
                    }
                    if (ok)
                        m += dir_obst[d];
                }
            }
            for (int d=0; d<4; d++)
            {
                int x = hx[i]+dir_x[d];
                int y = hy[i]+dir_y[d];
                if (0<=x && x<S && 0<=y && y<S &&
                    F[y][x]==0)
                    m += dir_move[d];
            }
            moves.push_back(m);
        }
        return moves;
    }

    void move(string humans, vector<string> pets)
    {
        for (int i=0; i<N; i++)
            for (int j=0; j<(int)pets[i].size(); j++)
                for (int d=0; d<4; d++)
                    if (pets[i][j]==dir_move[d])
                    {
                        int x = px[i];
                        int y = py[i];
                        P[y][x]--;
                        x += dir_x[d];
                        y += dir_y[d];
                        P[y][x]++;
                        px[i] = x;
                        py[i] = y;
                    }

        for (int i=0; i<M; i++)
        {
            for (int d=0; d<4; d++)
                if (humans[i]==dir_obst[d])
                {
                    int x = hx[i];
                    int y = hy[i];
                    x += dir_x[d];
                    y += dir_y[d];
                    F[y][x] = 1;
                }
            for (int d=0; d<4; d++)
                if (humans[i]==dir_move[d])
                {
                    int x = hx[i];
                    int y = hy[i];
                    H[y][x]--;
                    x += dir_x[d];
                    y += dir_y[d];
                    H[y][x]++;
                    hx[i] = x;
                    hy[i] = y;
                }
        }
        turn++;
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
    vector<int> hx(N), hy(N);
    for (int i=0; i<M; i++)
    {
        cin>>hx[i]>>hy[i];
        hx[i]--;
        hy[i]--;
    }

    Field field(px, py, pt, hx, hy);
    for (int t=0; t<T; t++)
    {
        vector<string> moves = field.get_human_moves();
        string human;
        for (int i=0; i<M; i++)
            human += moves[i][xor64()%(int)moves[i].size()];
        cout<<human<<endl;

        vector<string> pets(N);
        for (string &move: pets)
            cin>>move;

        field.move(human, pets);
    }
}
