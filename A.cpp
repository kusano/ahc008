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
        for (int x=0; x<S; x++)
            for (int y=0; y<S; y++)
                (*D)[x][y] = oo;
        Q.clear();

        (*D)[sx][sy] = 0;
        Q.push_back({sx, sy});
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
};

class AI
{
    int N, M;
    int gate_num;
    int current_gate;
    vector<int> states;
    vector<vector<int>> D;

public:
    AI(Field field):
        N(field.N), M(field.M)
    {
        gate_num = 1;
        for (int i=0; i<N; i++)
            if (field.pt[i]==3 || field.pt[i]==4)
                gate_num++;
        gate_num = min(gate_num, 14);
        // 犬猫がいないのでゲートを作る意味が無い
        if (gate_num==1)
            gate_num = 0;

        current_gate = gate_num-1;

        states = vector<int>(M);
        if (gate_num==0)
        {
            states[0] = 3;
            states[1] = 3;
            states[2] = 2;
            states[3] = 2;
            states[4] = 2;
        }

        D = vector<vector<int>>(S, vector<int>(S));
    }

    //  全員分の動きを返す
    vector<int> get_moves(Field field)
    {
        //  ゲートに捕獲するか
        bool capture = false;
        if (current_gate==0)
        {
            states[0] = 3;
            states[1] = 3;
        }
        if (states[0]==2 && states[1]==2)
        {
            //  ゲートに入りうる犬猫がいるかを確認
            bool ok = false;
            for (int i=0; i<N; i++)
                if (field.pt[i]==3 || field.pt[i]==4)
                    if (field.toward(field.px[i], field.py[i], S/2, current_gate*2)!=-1)
                        ok = true;
            if (!ok)
            {
                states[0] = 3;
                states[1] = 3;
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
            int x = field.hx[h];
            int y = field.hy[h];

            int move = 0;
            switch (h)
            {
            case 0:
                if (states[h]==0)
                {
                    move = field.toward(S/2-2, 2);
                    if (move==STAY)
                        states[h] = 1;
                }
                if (states[h]==1)
                {
                    move = get_moves_gate(field);
                    if (move==-1)
                        states[h] = 2;
                }
                if (states[h]==2)
                {
                    if (capture)
                        move = BLOCK_D;
                    else
                        move = field.toward(S/2-4, current_gate*2);
                }
                if (states[h]==3)
                    move = get_moves_chase(field);
                break;
            case 1:
                if (states[h]==0)
                {
                    move = field.toward(S/2+1, 2);
                    if (move==STAY)
                        states[h]=1;
                }
                if (states[h]==1)
                {
                    move = get_moves_gate(field);
                    if (move==-1)
                        states[h] = 2;
                }
                if (states[h]==2)
                {
                    if (capture)
                        move = BLOCK_U;
                    else
                        move = field.toward(S/2+3, current_gate*2);
                }
                if (states[h]==3)
                    move = get_moves_chase(field);
                break;
            case 2:
                if (states[h]==0)
                {
                    move = field.toward(S/2-1, 2);
                    if (move==STAY)
                        states[h]=1;
                }
                if (states[h]==1)
                {
                    move = get_moves_gate(field);
                    if (move==-1)
                        states[h] = 2;
                }
                if (states[h]==2)
                    move = get_moves_chase(field);
                break;
            case 3:
                if (states[h]==0)
                {
                    move = field.toward(S/2, 2);
                    if (move==8)
                        states[h]=1;
                }
                if (states[h]==1)
                {
                    move = get_moves_gate(field);
                    if (move==-1)
                        states[h] = 2;
                }
                if (states[h]==2)
                    move = get_moves_chase(field);
                break;
            case 4:
                if (states[h]==0)
                {
                    move = field.toward(S/2, S-2);
                    if (move==8)
                        states[h]=1;
                }
                if (states[h]==1)
                {
                    move = get_moves_fence(field);
                    if (move==-1)
                        states[h] = 2;
                }
                if (states[h]==2)
                    move = get_moves_chase(field);
                break;
            default:
                move = get_moves_chase(field);
                break;
            }

            field.move({move});
            moves[h] = move;
        }

        return moves;
    }

    //  ゲートを作る動きを返す。
    int get_moves_gate(Field &field)
    {
        int h = field.turn_sub;
        int x = field.hx[h];
        int y = field.hy[h];

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
            if (field.F[x-1][y]>0 && field.F[x][y+1]>0)
                if (y==gate_num*2)
                    return -1;
                else
                    return MOVE_L;
            else
                if (field.can_block(x-1, y))
                    return BLOCK_U;
                else if (field.can_block(x, y+1))
                    return BLOCK_R;
                else
                    return STAY;
    }

    //  追いかけて囲む
    int get_moves_chase(Field &field)
    {
        int h = field.turn_sub;
        int hx = field.hx[h];
        int hy = field.hy[h];

        field.get_distances(hx, hy, &D);

        //  目標選択
        //  犬猫以外で最も近いペット
        int target = -1;
        for (int p=0; p<N; p++)
            if (field.pt[p]!=3 && field.pt[p]!=4)
            {
                int x = field.px[p];
                int y = field.py[p];
                if (D[x][y]<oo &&
                    (target==-1 || D[x][y]<D[field.px[target]][field.py[target]]))
                    target = p;
            }
        if (target==-1)
            return STAY;

        int px = field.px[target];
        int py = field.py[target];
        int pd = D[px][py];

        field.get_distances(px, py, &D);

        vector<int> moves;

        if (pd<2)
        {
            //  離れる
            for (int d=0; d<4; d++)
            {
                int tx = hx+dir_x[d];
                int ty = hy+dir_y[d];
                if (0<=tx && tx<S && 0<=ty && ty<S &&
                    D[tx][ty]==pd+1 &&
                    field.F[tx][ty]==0)
                    moves.push_back(d);
            }
        }
        else if (pd==2 || pd==3)
        {
            //  通行不可にする
            for (int d=0; d<4; d++)
            {
                int tx = hx+dir_x[d];
                int ty = hy+dir_y[d];
                if (0<=tx && tx<S && 0<=ty && ty<S &&
                    D[tx][ty]==pd-1 &&
                    field.can_block(tx, ty))
                {
                    //  ゲート設置を邪魔しないか確認
                    bool ok = true;
                    if (gate_num>0)
                    {
                        if (S/2-4<=tx && tx<S/2+4 &&
                            (ty<current_gate*2+2))
                            ok = false;
                        if ((tx==S/2 || tx==S/2+1) ||
                            ty>=gate_num*2)
                            if (states[4]==0 ||
                                states[4]==1 && ty<=field.hy[4])
                                ok = false;
                    }
                    if (ok)
                    {
                        //  通行不可にしたことで人が閉じ込められるなら不可
                        field.F[tx][ty] = 1;
                        bool ok = true;
                        for (int h=0; h<M && ok; h++)
                            if (field.toward(field.hx[h], field.hy[h], S/2, 0)==-1)
                                ok = false;
                        field.F[tx][ty] = 0;
                        if (ok)
                        {
                            field.F[tx][ty] = 0;
                            moves.push_back(d+4);
                        }
                    }
                }
            }
        }
        else
        {
            // 近づく
            for (int d=0; d<4; d++)
            {
                int tx = hx+dir_x[d];
                int ty = hy+dir_y[d];
                if (0<=tx && tx<S && 0<=ty && ty<S &&
                    D[tx][ty]==pd-1 &&
                    field.F[tx][ty]==0)
                    moves.push_back(d);
            }
        }

        if (moves.empty())
            return STAY;
        else
            return moves[xor64()%(int)moves.size()];
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
}
