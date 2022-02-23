#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <functional>
#include <algorithm>
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

const int MOVE_U = 0;
const int MOVE_D = 1;
const int MOVE_L = 2;
const int MOVE_R = 3;
const int BLOCK_U = 4;
const int BLOCK_D = 5;
const int BLOCK_L = 6;
const int BLOCK_R = 7;
const int STAY = 8;
const string move_str = "UDLRudlr.";

const int dir_x[] = {-1, 1, 0, 0};
const int dir_y[] = {0, 0, -1, 1};

const int oo = 9999;

struct Field
{
    int N, M;
    vector<int> px, py, pt;
    vector<int> hx, hy;
    vector<int> hx_old, hy_old;

    int turn = 0;
    int turn_sub = 0;
    //  通行不可
    vector<vector<int>> F;
    //  人の数
    vector<vector<int>> H;
    //  ペットの数
    vector<vector<int>> P;
    //  距離計算に使用
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
    {
        hx_old.resize(M);
        hy_old.resize(M);
        F = H = P = D = vector<vector<int>>(S, vector<int>(S));

        for (int i=0; i<M; i++)
            H[hx[i]][hy[i]]++;
        for (int i=0; i<N; i++)
            P[px[i]][py[i]]++;
    }

    //  i番目の人を操作しようとしたとき、P[i]番目の人を操作するようにする。
    void permutate(vector<int> P)
    {
        vector<int> tmp_hx = hx;
        vector<int> tmp_hy = hy;
        vector<int> tmp_hx_old = hx_old;
        vector<int> tmp_hy_old = hy_old;
        for (int i=0; i<M; i++)
            for (int j=0; j<M; j++)
            {
                hx[i] = tmp_hx[P[i]];
                hy[i] = tmp_hy[P[i]];
                hx_old[i] = tmp_hx_old[P[i]];
                hy_old[i] = tmp_hy_old[P[i]];
            }
    }

    void move(vector<int> move)
    {
        if (turn_sub<M)
        {
            //  人
            int h = turn_sub;
            hx_old[h] = -1;
            int m = move[0];
            if (m<4)
            {
                //  移動
                int d = m;
                //  元いた場所も通行不可にはできないので、Hはターン終了時に減らす。
                hx_old[h] = hx[h];
                hy_old[h] = hy[h];
                hx[h] += dir_x[d];
                hy[h] += dir_y[d];
                H[hx[h]][hy[h]]++;
            }
            else if (m<8)
            {
                //  通行不可
                int d = m-4;
                int x = hx[h]+dir_x[d];
                int y = hy[h]+dir_y[d];
                F[x][y] = 1;
            }
        }
        else
        {
            //  ペット
            int p = turn_sub-M;
            for (int m: move)
            {
                int d = m;
                P[px[p]][py[p]]--;
                px[p] += dir_x[d];
                py[p] += dir_y[d];
                P[px[p]][py[p]]++;
            }
        }

        turn_sub++;
        if (turn_sub>=N+M)
        {
            for (int i=0; i<M; i++)
                if (hx_old[i]>=0)
                    H[hx_old[i]][hy_old[i]]--;

            turn_sub = 0;
            turn++;
        }
    }

    //  現在操作している人やペットが(x, y)に近づく動きを返す。
    //  (x, y)にいるならSTAY、到達不可能なら-1
    int toward(int x, int y)
    {
        if (turn_sub<M)
            return toward(hx[turn_sub], hy[turn_sub], x, y);
        else
            return toward(px[turn_sub-M], py[turn_sub-M], x, y);
    }

    //  (sx, sy)から(gx, gy)に向かう動きを返す。
    int toward(int sx, int sy, int gx, int gy)
    {
        if (sx==gx && sy==gy)
            return STAY;

        get_distances(gx, gy, &D);

        if (D[sx][sy]==oo)
            return -1;

        vector<int> moves;
        for (int d=0; d<4; d++)
        {
            int tx = sx+dir_x[d];
            int ty = sy+dir_y[d];
            if (0<=tx && tx<S && 0<=ty && ty<S &&
                D[tx][ty]==D[sx][sy]-1)
                moves.push_back(d);
        }

        return moves[xor64()%(int)moves.size()];
    }

    bool can_block(int x, int y)
    {
        if (F[x][y]!=0 || H[x][y]!=0 || P[x][y]!=0)
            return false;
        for (int d=0; d<4; d++)
        {
            int tx = x+dir_x[d];
            int ty = y+dir_y[d];
            if (0<=tx && tx<S && 0<=ty && ty<S &&
                P[tx][ty]>0)
                return false;
        }
        return true;
    }

    // (sx, sy)からの距離を求める
    void get_distances(int sx, int sy, vector<vector<int>> *D)
    {
        get_distances(vector<int>{sx}, vector<int>{sy}, D);
    }

    void get_distances(vector<int> SX, vector<int> SY, vector<vector<int>> *D)
    {
        for (int x=0; x<S; x++)
            for (int y=0; y<S; y++)
                (*D)[x][y] = oo;
        Q.clear();

        for (int i=0; i<(int)SX.size(); i++)
        {
            (*D)[SX[i]][SY[i]] = 0;
            Q.push_back({SX[i], SY[i]});
        }
        for (int q=0; q<(int)Q.size(); q++)
        {
            int x = Q[q].first;
            int y = Q[q].second;
            for (int d=0; d<4; d++)
            {
                int tx = x+dir_x[d];
                int ty = y+dir_y[d];
                if (0<=tx && tx<S && 0<=ty && ty<S &&
                    F[tx][ty]==0 &&
                    (*D)[tx][ty]==oo)
                {
                    (*D)[tx][ty] = (*D)[x][y]+1;
                    Q.push_back({tx, ty});
                }
            }
        }
    }

    long long score()
    {
        for (int y=0; y<S; y++)
            for (int x=0; x<S; x++)
                D[x][y] = 0;

        long long score = 0;
        for (int i=0; i<M; i++)
        {
            int ox = hx[i];
            int oy = hy[i];
            if (D[ox][oy]==0)
            {
                int cn = 0;
                int hn = 0;
                int pn = 0;

                Q.clear();
                Q.push_back({ox, oy});
                for (int q=0; q<(int)Q.size(); q++)
                {
                    int x = Q[q].first;
                    int y = Q[q].second;

                    if (D[x][y]==1)
                        continue;
                    D[x][y] = 1;
                    cn++;
                    hn += H[x][y];
                    pn += P[x][y];

                    for (int d=0; d<4; d++)
                    {
                        int tx = x+dir_x[d];
                        int ty = y+dir_y[d];
                        if (0<=tx && tx<S && 0<=ty && ty<S &&
                            F[tx][ty]==0 &&
                            D[tx][ty]==0)
                            Q.push_back({tx, ty});
                    }
                }

                score += (long long)(cn*hn)<<(N-pn);
            }
        }
        long long denom = (long long)(S*S*M)<<N;
        score = (score*100000000+denom/2)/denom;
        return score;
    }
};

class AI
{
    static const int STATE_PREPARE = 0;     //  ゲート設置の開始位置に向かう
    static const int STATE_GATE    = 1;     //  ゲート設置
    static const int STATE_TRAP    = 2;     //  ゲートにペットを閉じ込める
    static const int STATE_CHASE   = 3;     //  ペットを追う

    int N, M;
    int gate_num;
    int current_gate;
    //  人iは実際には人P[i]
    vector<int> P;
    vector<int> states;
    vector<vector<int>> D, D2;
    //  上半分／下半分で狙うペット
    int target_up = -1;
    int target_down = -1;

public:
    AI(Field field):
        N(field.N), M(field.M)
    {
        gate_num = 1;
        for (int i=0; i<N; i++)
            if (field.pt[i]==3 || field.pt[i]==4)
                gate_num++;
        gate_num = min(gate_num, 14);

        current_gate = gate_num-1;

        //  人を割り当てる。
        //  ゲートができるまで機能しないので、途中から離れる人を優先する。
        vector<int> idx{4,     2,     3,     0,     1    };
        vector<int> gx {S/2-1, S/2-2, S/2+1, S/2-1, S/2  };
        vector<int> gy {S-2,   2,     2,     2,     2    };
        vector<bool> U(M);
        P.resize(5);
        for (int i=0; i<5; i++)
        {
            int dmin = oo;
            int h;
            for (int j=0; j<M; j++)
                if (!U[j])
                {
                    int d = abs(field.hx[j]-gx[i])+abs(field.hy[j]-gy[i]);
                    if (d<dmin)
                    {
                        dmin = d;
                        h = j;
                    }
                }
            P[idx[i]] = h;
            U[h] = true;
        }
        //  残りの人は、上に近い人を偶数、下に近い人を奇数に振り分ける。
        vector<pair<int, int>> V;
        for (int i=0; i<M; i++)
            if (!U[i])
                V.push_back({field.hx[i], i});
        sort(V.begin(), V.end());
        while (!V.empty())
        {
            P.push_back(V.back().second);
            V.pop_back();
            if (!V.empty())
            {
                P.push_back(V.front().second);
                V.erase(V.begin());
            }
        }

        for (int i=0; i<M; i++)
        {
            bool ok = true;
            for (int x: P)
                if (x==i)
                    ok = false;
            if (ok)
                P.push_back(i);
        }

        states = vector<int>(M);
        for (int h=0; h<5; h++)
            states[0] = STATE_PREPARE;
        for (int h=5; h<M; h++)
            states[h] = STATE_CHASE;

        D = D2 = vector<vector<int>>(S, vector<int>(S));
    }

    //  全員分の動きを返す
    vector<int> get_moves(Field field)
    {
        field.permutate(P);

        //  ゲートに捕獲するか
        bool capture = false;
        if (current_gate==0 &&
            field.F[S/2-2][1]==1 &&
            field.F[S/2-1][1]==1 &&
            field.F[S/2  ][1]==1 &&
            field.F[S/2+1][1]==1)
        {
            states[0] = STATE_CHASE;
            states[1] = STATE_CHASE;
        }
        if (states[0]==STATE_TRAP &&
            states[1]==STATE_TRAP &&
            states[2]==STATE_CHASE &&
            states[3]==STATE_CHASE)
        {
            //  ゲートに入りうる犬猫がいるかを確認
            bool ok = false;
            for (int i=0; i<N; i++)
                if (field.pt[i]==3 || field.pt[i]==4)
                    if (field.toward(field.px[i], field.py[i], S/2, current_gate*2)!=-1)
                        ok = true;
            if (!ok)
            {
                states[0] = STATE_CHASE;
                states[1] = STATE_CHASE;
                current_gate = 0;
            }
            else
                if (field.hx[0]==S/2-4 && field.hy[0]==current_gate*2 &&
                    field.hx[1]==S/2+3 && field.hy[1]==current_gate*2 &&
                    field.can_block(S/2-3, current_gate*2) &&
                    field.can_block(S/2+2, current_gate*2))
                {
                    //  現在のゲート内に犬猫がいるか
                    bool ok = false;
                    for (int i=0; i<N; i++)
                        if (field.pt[i]==3 || field.pt[i]==4)
                            if ((field.px[i]==S/2-1 || field.px[i]==S/2) &&
                                field.py[i]==current_gate*2)
                                ok = true;
                    if (ok)
                    {
                        //  現在のゲート内に人がいないか
                        bool ok = true;
                        for (int i=0; i<M; i++)
                            if (field.hy[i]==current_gate*2 &&
                                S/2-2<=field.hx[i] && field.hx[i]<S/2+2)
                                ok = false;
                        if (ok)
                        {
                            capture = true;
                            current_gate--;
                        }
                    }
                }
        }

        vector<int> moves(M);

        for (int h=0; h<M; h++)
        {
            int move = -1;
            if (states[h]==STATE_PREPARE)
            {
                switch (h)
                {
                case 0: move = field.toward(S/2-1, 2  ); break;
                case 1: move = field.toward(S/2 ,  2  ); break;
                case 2: move = field.toward(S/2-2, 2  ); break;
                case 3: move = field.toward(S/2+1, 2  ); break;
                case 4: move = field.toward(S/2-1, S-2); break;
                }
                if (move==STAY)
                    states[h] = STATE_GATE;
            }
            if (states[h]==STATE_GATE)
            {
                if (h<4)
                    move = get_moves_gate(field);
                else
                    move = get_moves_fence(field);
                if (move==-1)
                {
                    if (h==0 || h==1)
                        states[h] = STATE_TRAP;
                    else
                        states[h] = STATE_CHASE;
                }
            }
            if (states[h]==STATE_TRAP)
            {
                if (capture)
                    if (h==0)
                        move = BLOCK_D;
                    else
                        move = BLOCK_U;
                else
                {
                    if (h==0)
                        move = field.toward(S/2-4, current_gate*2);
                    else
                        move = field.toward(S/2+3, current_gate*2);
                    if (move==-1)
                        move = STAY;
                }
            }
            if (states[h]==STATE_CHASE)
            {
                move = get_moves_chase(field);
            }

            //  最後16ターンはスコアを上げられるなら上げる
            if (field.turn>=T-16)
            {
                int m = get_moves_score_up(field);
                if (m!=STAY)
                    move = m;
            }

            field.move({move});
            moves[h] = move;
        }

        //  並べ替え
        vector<int> moves2(M);
        for (int i=0; i<M; i++)
            moves2[P[i]] = moves[i];
        moves = moves2;

        return moves;
    }

    //  ゲートを作る動きを返す。
    int get_moves_gate(Field &field)
    {
        int h = field.turn_sub;
        int x = field.hx[h];
        int y = field.hy[h];

        if (gate_num==1)
            if (field.F[x][y-1]>0)
                return -1;
            else
                if (field.can_block(x, y-1))
                    return BLOCK_L;
                else
                    return STAY;
        if (y==gate_num*2-2)
            if (field.F[x][y-1]>0 && field.F[x][y+1]>0)
                return -1;
            else
                if (field.can_block(x, y-1))
                    return BLOCK_L;
                else if (field.can_block(x, y+1))
                    return BLOCK_R;
                else
                    return STAY;
        else
            if (y%2==0)
                if (field.F[x][y-1]>0)
                    return MOVE_R;
                else
                    if (field.can_block(x, y-1))
                        return BLOCK_L;
                    else
                        return STAY;
            else
                return MOVE_R;
    }

    //  右側の壁を作る動きを返す。
    int get_moves_fence(Field &field)
    {
        int h = field.turn_sub;
        int x = field.hx[h];
        int y = field.hy[h];

        if (y%2!=0)
            return MOVE_L;
        else
            if (field.F[x][y+1]>0 && field.F[x+1][y]>0)
                if (y==gate_num*2)
                    return -1;
                else
                    return MOVE_L;
            else
                if (field.can_block(x, y+1))
                    return BLOCK_R;
                else if (field.can_block(x+1, y))
                    return BLOCK_D;
                else
                    return STAY;
    }

    //  追いかけて囲む
    int get_moves_chase(Field &field)
    {
        int h = field.turn_sub;
        int hx = field.hx[h];
        int hy = field.hy[h];

        //  目標選択
        /*
        //  偶数番目の人は上半分、奇数番目の人は下半分のペット。
        //  偶数番目の人は右上、奇数番目は右下に近いペットを優先する。
        //  上半分／下半分の全てのペットが捕獲済みなら、下半分／上半分に向かう。
        int &target = h%2==0 ? target_up : target_down;

        field.get_distances(S/2, 0, &D);

        if (target!=-1 &&
            D[field.px[target]][field.py[target]]==oo)
            target = -1;
        if (target==-1)
        {
            for (int i=0; i<2 && target==-1; i++)
            {
                int up_down;
                if (i==0 && h%2==0 ||
                    i==1 && h%2!=0)
                    up_down = 0;
                else
                    up_down = 1;

                int dmin = oo;
                for (int p=0; p<N; p++)
                {
                    int x = field.px[p];
                    int y = field.py[p];
                    //  犬猫はゲートで捕まえるので狙わない。
                    if ((field.pt[p]!=3 && field.pt[p]!=4) &&
                        D[x][y]<oo &&
                        (up_down==0 && x<S/2 ||
                         up_down==1 && x>=S/2))
                    {
                        int d = abs(x-(S-1)*up_down)+abs(y-(S-1));
                        if (d<dmin)
                        {
                            dmin = d;
                            target = p;
                        }
                    }
                }
            }
        }
        */

        //  上半分／下半分で最も近いペット
        int target = -1;

        field.get_distances(hx, hy, &D);

        for (int i=0; i<2 && target==-1; i++)
        {
            int up_down;
            if (i==0 && h%2==0 ||
                i==1 && h%2!=0)
                up_down = 0;
            else
                up_down = 1;

            int dmin = oo;
            for (int p=0; p<N; p++)
            {
                int x = field.px[p];
                int y = field.py[p];
                //  犬猫はゲートで捕まえるので狙わない。
                if ((field.pt[p]!=3 && field.pt[p]!=4) &&
                    (up_down==0 && x<S/2 ||
                     up_down==1 && x>=S/2) &&
                    D[x][y]<dmin)
                {
                    dmin = D[x][y];
                    target = p;
                }
            }
        }

        if (target==-1)
            return STAY;

        field.get_distances(S/2, 0, &D);

        int px = field.px[target];
        int py = field.py[target];
        int pd = D[px][py];

        field.get_distances(px, py, &D2);

        vector<int> moves;

        //  (SX/2, 0)から(px, py)の最短経路上で、(px, py)からの距離が2の位置を、
        //  通行不可にできるならする。
        for (int d=0; d<4; d++)
        {
            int tx = hx+dir_x[d];
            int ty = hy+dir_y[d];
            if (0<=tx && tx<S && 0<=ty && ty<S &&
                D[tx][ty]==pd-2 &&
                D2[tx][ty]==2 &&
                field.can_block(tx, ty))
            {
                //  ゲート設置を邪魔しないか確認
                bool ok = true;
                if (S/2-4<=tx && tx<S/2+4 &&
                    ty<current_gate*2+2)
                    ok = false;
                if (tx==S/2-1 || tx==S/2)
                    ok = false;
                if (tx==S/2-2 &&
                    gate_num*2<=ty)
                    if (states[4]==0 ||
                        states[4]==1 && ty<=field.hy[4])
                        ok = false;
                if (ok)
                {
                    //  通行不可にしたことで人が閉じ込められるなら不可
                    field.get_distances(S/2, 0, &D);
                    field.F[tx][ty] = 1;
                    field.get_distances(S/2, 0, &D2);
                    field.F[tx][ty] = 0;
                    bool ok = true;
                    for (int h=0; h<M && ok; h++)
                        if (D[field.hx[h]][field.hy[h]]<oo &&
                            D2[field.hx[h]][field.hy[h]]==oo)
                            ok = false;
                    if (ok)
                        moves.push_back(d+4);
                }
            }
        }

        field.get_distances(S/2, 0, &D);
        field.get_distances(px, py, &D2);

        if (moves.empty())
        {
            if (D[hx][hy]==pd-2 &&
                D2[hx][hy]==2)
                //  距離2のマスにいるなら移動しない
                ;
            else
            {
                //  距離3のマスに近づく
                vector<int> SX, SY;
                for (int x=0; x<S; x++)
                    for (int y=0; y<S; y++)
                        if (D[x][y]==pd-3 &&
                            D2[x][y]==3)
                        {
                            SX.push_back(x);
                            SY.push_back(y);
                        }

                if (!SX.empty())
                {
                    field.get_distances(SX, SY, &D);

                    for (int d=0; d<4; d++)
                    {
                        int tx = hx+dir_x[d];
                        int ty = hy+dir_y[d];
                        if (0<=tx && tx<S && 0<=ty && ty<S &&
                            D[tx][ty]==D[hx][hy]-1 &&
                            field.F[tx][ty]==0)
                            moves.push_back(d);
                    }
                }
            }
        }

        if (moves.empty())
            return STAY;
        else
            return moves[xor64()%(int)moves.size()];
    }

    //  通行不可にすることでスコアを上げられるなら、通行不可にする
    int get_moves_score_up(Field &field)
    {
        int h = field.turn_sub;
        int hx = field.hx[h];
        int hy = field.hy[h];

        long long score = field.score();
        int move = STAY;

        for (int d=0; d<4; d++)
        {
            int tx = hx+dir_x[d];
            int ty = hy+dir_y[d];
            if (0<=tx && tx<S && 0<=ty && ty<S &&
                field.can_block(tx, ty))
            {
                field.F[tx][ty] = 1;
                long long s = field.score();
                field.F[tx][ty] = 0;
                if (s>score)
                {
                    score = s;
                    move = d+4;
                }
            }
        }

        return move;
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
    AI ai(field);

    for (int t=0; t<T; t++)
    {
        vector<int> moves = ai.get_moves(field);

        string ans;
        for (int m: moves)
        {
            field.move({m});
            ans += move_str[m];
        }
        cout<<ans<<endl;

        for (int i=0; i<N; i++)
        {
            string move;
            cin>>move;
            vector<int> movei;
            for (char m: move)
                for (int d=0; d<4; d++)
                    if (move_str[d]==m)
                        movei.push_back(d);
            field.move(movei);
        }
    }

    fprintf(stderr, "Number of humans: %2d\n", M);
    int PC[5] = {};
    for (int i=0; i<N; i++)
        PC[field.pt[i]]++;
    fprintf(stderr, "Number of pets:   %2d (%2d, %2d, %2d, %2d, %2d)\n",
        N, PC[0], PC[1], PC[2], PC[3], PC[4]);
    int UPC[5] = {};
    int s = 0;
    for (int p=0; p<N; p++)
        if (field.toward(field.px[p], field.py[p], S/2, 0)!=-1)
        {
            UPC[field.pt[p]]++;
            s++;
        }
    fprintf(stderr, "Uncaptured pets:  %2d (%2d, %2d, %2d, %2d, %2d)\n",
        s, UPC[0], UPC[1], UPC[2], UPC[3], UPC[4]);
    long long score = field.score();
    fprintf(stderr, "Score: %7.4f %%\n", score*1e-8*100);
}
