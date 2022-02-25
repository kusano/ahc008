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

const int TYPE_COW = 0;
const int TYPE_PIG = 1;
const int TYPE_RABBIT = 2;
const int TYPE_DOG = 3;
const int TYPE_CAT = 4;

const int dir[] = {-S, S, -1, 1};

const int oo = 9999;

struct Field
{
    int N, M;
    vector<int> pp, pt;
    vector<int> hp;
    vector<int> hp_old;

    int turn = 0;
    int turn_sub = 0;
    //  通行不可
    vector<int> F;
    //  人の数
    vector<int> H;
    //  ペットの数
    vector<int> P;
    //  距離計算に使用
    vector<int> D;
    vector<int> Q;

    Field(vector<int> pp, vector<int> pt, vector<int> hp)
        : N(int(pp.size()))
        , M(int(hp.size()))
        , pp(pp)
        , pt(pt)
        , hp(hp)
    {
        hp_old.resize(M);
        F = H = P = D = vector<int>(S*S);

        for (int h=0; h<M; h++)
            H[hp[h]]++;
        for (int p=0; p<N; p++)
            P[pp[p]]++;
    }

    //  i番目の人を操作しようとしたとき、P[i]番目の人を操作するようにする。
    void permutate(vector<int> P)
    {
        vector<int> tmp_hp = hp;
        vector<int> tmp_hp_old = hp_old;
        for (int i=0; i<M; i++)
        {
            hp[i] = tmp_hp[P[i]];
            hp_old[i] = tmp_hp_old[P[i]];
        }
    }

    void move(vector<int> move)
    {
        if (turn_sub<M)
        {
            //  人
            int h = turn_sub;
            hp_old[h] = -1;
            int m = move[0];
            if (m<4)
            {
                //  移動
                int d = m;
                //  元いた場所も通行不可にはできないので、Hはターン終了時に減らす。
                hp_old[h] = hp[h];
                hp[h] += dir[d];
                H[hp[h]]++;
            }
            else if (m<8)
            {
                //  通行不可
                int d = m-4;
                int p = hp[h]+dir[d];
                F[p] = 1;
            }
        }
        else
        {
            //  ペット
            int p = turn_sub-M;
            for (int m: move)
            {
                int d = m;
                P[pp[p]]--;
                pp[p] += dir[d];
                P[pp[p]]++;
            }
        }

        turn_sub++;
        if (turn_sub>=N+M)
        {
            for (int i=0; i<M; i++)
                if (hp_old[i]>=0)
                    H[hp_old[i]]--;

            turn_sub = 0;
            turn++;
        }
    }

    //  現在操作している人やペットがposに近づく動きを返す。
    //  posにいるならSTAY、到達不可能なら-1
    int toward(int pos)
    {
        if (turn_sub<M)
            return toward(hp[turn_sub], pos);
        else
            return toward(pp[turn_sub-M], pos);
    }

    //  spからgpに向かう動きを返す。
    int toward(int sp, int gp)
    {
        if (sp==gp)
            return STAY;

        if (F[gp]!=0)
            return -1;

        get_distances(gp, &D);

        if (D[sp]==oo)
            return -1;

        vector<int> moves;
        for (int d=0; d<4; d++)
            if (can_move(sp, d))
            {
                int tp = sp+dir[d];
                if (D[tp]==D[sp]-1)
                    moves.push_back(d);
            }

        return moves[xor64()%(int)moves.size()];
    }

    //  posから動きdができるかを返す
    bool can_move(int pos, int d)
    {
        switch (d)
        {
        case MOVE_U: return pos/S>=1;
        case MOVE_D: return pos/S<S-1;
        case MOVE_L: return pos%S>=1;
        case MOVE_R: return pos%S<S-1;
        }
        return false;
    }

    bool can_block(int pos)
    {
        if (F[pos]!=0 || H[pos]!=0 || P[pos]!=0)
            return false;
        for (int d=0; d<4; d++)
            if (can_move(pos, d))
            {
                int t = pos+dir[d];
                if (P[t]>0)
                    return false;
            }
        return true;
    }

    // spからの距離を求める
    void get_distances(int sp, vector<int> *D)
    {
        get_distances(vector<int>{sp}, D);
    }

    void get_distances(vector<int> SP, vector<int> *D)
    {
        for (int &d: *D)
            d = oo;
        Q.clear();

        for (int p: SP)
        {
            (*D)[p] = 0;
            Q.push_back(p);
        }

        for (int q=0; q<(int)Q.size(); q++)
        {
            int p = Q[q];
            for (int d=0; d<4; d++)
                if (can_move(p, d))
                {
                    int tp = p+dir[d];
                    if (F[tp]==0 && (*D)[tp]==oo)
                    {
                        (*D)[tp] = (*D)[p]+1;
                        Q.push_back(tp);
                    }
                }
        }
    }

    //  spから到達可能なマス数を返す
    int get_area(int sp)
    {
        get_distances(sp, &D);
        int c = 0;
        for (int d: D)
            if (d<oo)
                c++;
        return c;
    }

    long long score()
    {
        for (int &d: D)
            d = 0;

        long long score = 0;
        for (int i=0; i<M; i++)
        {
            int op = hp[i];
            if (D[op]==0)
            {
                int cn = 0;
                int hn = 0;
                int pn = 0;

                Q.clear();
                Q.push_back(op);
                for (int q=0; q<(int)Q.size(); q++)
                {
                    int p = Q[q];

                    if (D[p]==1)
                        continue;
                    D[p] = 1;
                    cn++;
                    hn += H[p];
                    pn += P[p];

                    for (int d=0; d<4; d++)
                        if (can_move(p, d))
                        {
                            int tp = p+dir[d];
                            if (F[tp]==0 && D[tp]==0)
                                Q.push_back(tp);
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

struct AI
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
    vector<int> D1, D2, D3, D4;
    //  上半分／下半分で狙うペット
    int target_up = -1;
    int target_down = -1;
    vector<int> targets;
    vector<int> targets_up_down;

    AI(Field field):
        N(field.N), M(field.M)
    {
        gate_num = 1;
        for (int i=0; i<N; i++)
            if (field.pt[i]==TYPE_DOG ||
                field.pt[i]==TYPE_CAT)
                gate_num++;
        gate_num = min(gate_num, 14);

        if (gate_num<=1)
            gate_num = 0;

        current_gate = gate_num-1;

        if (gate_num>0)
        {
            //  人を割り当てる。
            //  ゲートができるまで機能しないので、途中から離れる人を優先する。
            vector<int> idx{4,               2,           3,           0,           1          };
            vector<int> gp {(S/2-1)*S+(S-2), (S/2-2)*S+2, (S/2+1)*S+2, (S/2-1)*S+2, (S/2  )*S+2};
            vector<bool> U(M);
            P.resize(5);
            for (int i=0; i<5; i++)
            {
                int dmin = oo;
                int h;
                for (int j=0; j<M; j++)
                    if (!U[j])
                    {
                        int d = abs(field.hp[j]/S-gp[i]/S)+abs(field.hp[j]%S-gp[i]%S);
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
                    V.push_back({field.hp[i]/S, i});
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
        }
        else
        {
            P = vector<int>(M);
            states = vector<int>(M);
            for (int h=0; h<M; h++)
            {
                P[h] = h;
                states[h] = STATE_CHASE;
            }
        }

        D1 = D2 = D3 = D4 = vector<int>(S*S);

        targets = vector<int>(M, -1);
        targets_up_down = vector<int>(M, -1);
    }

    //  全員分の動きを返す
    vector<int> get_moves(Field field)
    {
        field.permutate(P);

        //  ゲートに捕獲するか
        bool capture = false;
        if (current_gate==0 &&
            field.F[(S/2-2)*S+1]==1 &&
            field.F[(S/2-1)*S+1]==1 &&
            field.F[(S/2  )*S+1]==1 &&
            field.F[(S/2+1)*S+1]==1)
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
                if (field.pt[i]==TYPE_DOG ||
                    field.pt[i]==TYPE_CAT)
                    if (field.toward(field.pp[i], S/2*S+current_gate*2)!=-1)
                        ok = true;
            if (!ok)
            {
                states[0] = STATE_CHASE;
                states[1] = STATE_CHASE;
                current_gate = 0;
            }
            else
                if (field.hp[0]==(S/2-4)*S+current_gate*2 &&
                    field.hp[1]==(S/2+3)*S+current_gate*2 &&
                    field.can_block((S/2-3)*S+current_gate*2) &&
                    field.can_block((S/2+2)*S+current_gate*2))
                {
                    //  現在のゲート内に犬猫がいるか
                    bool ok = false;
                    for (int i=0; i<N; i++)
                        if (field.pt[i]==TYPE_DOG ||
                            field.pt[i]==TYPE_CAT)
                            if (field.pp[i]==(S/2-1)*S+current_gate*2 ||
                                field.pp[i]==(S/2  )*S+current_gate*2)
                                ok = true;
                    if (ok)
                    {
                        //  現在のゲート内に人がいないか
                        bool ok = true;
                        for (int i=0; i<M; i++)
                            if (field.hp[i]%S==current_gate*2 &&
                                S/2-2<=field.hp[i]/S && field.hp[i]/S<S/2+2)
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
                case 0: move = field.toward((S/2-1)*S+2); break;
                case 1: move = field.toward((S/2  )*S+2); break;
                case 2: move = field.toward((S/2-2)*S+2); break;
                case 3: move = field.toward((S/2+1)*S+2); break;
                case 4: move = field.toward((S/2-1)*S+(S-2)); break;
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
                        move = field.toward((S/2-4)*S+current_gate*2);
                    else
                        move = field.toward((S/2+3)*S+current_gate*2);
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
        int p = field.hp[h];

        if (gate_num==1)
            if (field.F[p-1]>0)
                return -1;
            else
                if (field.can_block(p-1))
                    return BLOCK_L;
                else
                    return STAY;
        if (p%S==gate_num*2-2)
            if (field.F[p-1]>0 && field.F[p+1]>0)
                return -1;
            else
                if (field.can_block(p-1))
                    return BLOCK_L;
                else if (field.can_block(p+1))
                    return BLOCK_R;
                else
                    return STAY;
        else
            if (p%S%2==0)
                if (field.F[p-1]>0)
                    return MOVE_R;
                else
                    if (field.can_block(p-1))
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
        int p = field.hp[h];

        if (p%2%2!=0)
            return MOVE_L;
        else
            if (field.F[p+1]>0 && field.F[p+S]>0)
                if (p%S==gate_num*2)
                    return -1;
                else
                    return MOVE_L;
            else
                if (field.can_block(p+1))
                    return BLOCK_R;
                else if (field.can_block(p+S))
                    return BLOCK_D;
                else
                    return STAY;
    }

    //  追いかけて囲む
    int get_moves_chase(Field &field)
    {
        int h = field.turn_sub;
        int hp = field.hp[h];

        //  目標選択
        /*
        //  偶数番目の人は上半分、奇数番目の人は下半分のペット。
        //  偶数番目の人は右上、奇数番目は右下に近いペットを優先する。
        //  上半分／下半分の全てのペットが捕獲済みなら、下半分／上半分に向かう。
        int &target = h%2==0 ? target_up : target_down;

        field.get_distances(S/2*S+0, &D1);

        if (target!=-1 &&
            D1[field.pp[target]]==oo)
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
                    int pp = field.pp[p];
                    //  犬猫はゲートで捕まえるので狙わない。
                    if ((field.pt[p]!=TYPE_DOG &&
                         field.pt[p]!=TYPE_CAT) &&
                        D1[pp]<oo &&
                        (up_down==0 && pp/S<S/2 ||
                         up_down==1 && pp/S>=S/2))
                    {
                        int d = abs(pp/S-(S-1)*up_down)+abs(pp%S-(S-1));
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

        /*
        //  上半分／下半分で最も近いペット
        int target = -1;

        field.get_distances(hp, &D1);

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
                int pp = field.pp[p];
                //  犬猫はゲートで捕まえるので狙わない。
                if ((field.pt[p]!=TYPE_DOG &&
                     field.pt[p]!=TYPE_CAT) &&
                    (up_down==0 && pp/S<S/2 ||
                     up_down==1 && pp/S>=S/2) &&
                    D1[pp]<dmin)
                {
                    dmin = D1[pp];
                    target = p;
                }
            }
        }
        */

        //  中央からの距離が遠いペット
        //  人ごとに違うペットを狙う。
        int &target = targets[h];

        field.get_distances(S/2*S+0, &D1);

        if (gate_num>0)
        {
            if (target!=-1)
            {
                //  ペットが閉じ込められた。
                if (D1[field.pp[target]]==oo)
                    target = -1;

                //  ペットが下半分／上半分に逃げた。
                if (targets_up_down[h]==0 && field.pp[target]/S>=S/2 ||
                    targets_up_down[h]==1 && field.pp[target]/S<S/2)
                    target = -1;
            }

            if (target==-1)
            {
                //  残りが猫だけになったら、猫も追う。
                bool cat_only = false;
                for (int p=0; p<N; p++)
                    if (field.pt[p]==TYPE_CAT &&
                        D1[field.pp[p]]<oo)
                        cat_only = true;
                for (int p=0; p<N; p++)
                    if (field.pt[p]!=TYPE_CAT &&
                        D1[field.pp[p]]<oo)
                        cat_only = false;

                for (int i=0; i<2 && target==-1; i++)
                {
                    ////  犬を誘導するため、捕まっていない犬がいるなら、もう半分にはいかない
                    //if (i==1)
                    //{
                    //    bool ok = true;
                    //    for (int p=0; p<N; p++)
                    //        if (field.pt[p]==TYPE_DOG &&
                    //            D1[field.pp[p]]<oo)
                    //            ok = false;
                    //    if (!ok)
                    //        continue;
                    //}

                    for (int j=0; j<2 && target==-1; j++)
                    {
                        int up_down;
                        if (i==0 && h%2==0 ||
                            i==1 && h%2!=0)
                            up_down = 0;
                        else
                            up_down = 1;

                        int dmax = 0;
                        for (int p=0; p<N; p++)
                        {
                            //  まずは他の人と違うペットを狙う
                            if (j==0)
                            {
                                bool ok = true;
                                for (int t: targets)
                                    if (t==p)
                                        ok = false;
                                if (!ok)
                                    continue;
                            }

                            int pp = field.pp[p];
                            //  犬猫はゲートで捕まえるので狙わない。
                            //  ゲートを使い切っていれば、狙う。
                            if (((current_gate==0 || field.pt[p]!=TYPE_DOG) &&
                                 (current_gate==0 || cat_only || field.pt[p]!=TYPE_CAT)) &&
                                (up_down==0 && pp/S<S/2 ||
                                 up_down==1 && pp/S>=S/2) &&
                                D1[pp]<oo &&
                                D1[pp]>dmax)
                            {
                                dmax = D1[pp];
                                target = p;
                                targets_up_down[h] = up_down;
                            }
                        }
                    }
                }
            }

            if (target==-1)
            {
                //  犬の誘導のため、ゲートを閉める人の待機場所近くに行く。
                //  犬が多いとゲートが詰まるので、待機場所の少し右。
                int m = -1;
                for (int y=min(S-1, current_gate*2+6); y>=current_gate*2 && m==-1; y--)
                {
                    if (h%2==0)
                        m = field.toward((S/2-4)*S+y);
                    else
                        m = field.toward((S/2+3)*S+y);
                }
                if (m==-1)
                    m = STAY;
                return m;
            }
        }
        else
        {
            //  ゲートを作らないとき
            if (target!=-1 &&
                D1[field.pp[target]]==oo)
                target = -1;

            if (target==-1)
            {
                for (int i=0; i<2 && target==-1; i++)
                {
                    for (int j=0; j<2 && target==-1; j++)
                    {
                        int dmax = 0;
                        for (int p=0; p<N; p++)
                        {
                            //  まずは他の人と違うペットを狙う
                            if (i==0)
                            {
                                bool ok = true;
                                for (int t: targets)
                                    if (t==p)
                                        ok = false;
                                if (!ok)
                                    continue;
                            }

                            //  犬猫は後回し
                            if (j==0 &&
                                (field.pt[p]==TYPE_DOG || field.pt[p]==TYPE_CAT))
                                continue;

                            int pp = field.pp[p];
                            if (D1[pp]<oo &&
                                D1[pp]>dmax)
                            {
                                dmax = D1[pp];
                                target = p;
                            }
                        }
                    }
                }
            }

            if (target==-1)
                return STAY;
        }

        field.get_distances(S/2*S+0, &D1);

        int pp = field.pp[target];
        int pd = D1[pp];

        field.get_distances(pp, &D2);

        vector<int> moves;

        //  通行不可にする動き
        for (int d=0; d<4; d++)
            if (field.can_move(hp, d))
            {
                int tp = hp+dir[d];
                if (field.can_block(tp))
                {
                    if (gate_num>0)
                    {
                        //  ゲート設置を邪魔しないか確認
                        bool ok = true;
                        if (S/2-4<=tp/S && tp/S<S/2+4 &&
                            tp%S<current_gate*2+2)
                            ok = false;
                        if (tp/S==S/2-1 || tp/S==S/2)
                            ok = false;
                        // 先に設置してしまうと、ゲートによって隔離される可能性があるので、
                        // 下側もゲート設置完了までは置かない。
                        if ((tp/S==S/2-2 || tp/S==S/2+1) &&
                            gate_num*2<=tp%S)
                            if (states[4]==0 ||
                                states[4]==1 && tp%S<=field.hp[4]%S)
                                ok = false;
                        if (!ok)
                            continue;
                    }

                    field.F[tp] = 1;
                    field.get_distances(S/2*S+0, &D3);
                    field.F[tp] = 0;

                    //  通行不可にしたことで人が閉じ込められるなら不可
                    bool ok = true;
                    for (int h=0; h<M && ok; h++)
                        if (D1[field.hp[h]]<oo &&
                            D3[field.hp[h]]==oo)
                            ok = false;
                    if (!ok)
                        continue;

                    //  通行不可にすることで、新たにペットを閉じ込めることができ、
                    //  ペット1匹あたりのマス数が30以下なら通行不可にする。
                    //  複数の方向で条件を満たすかもしれないので、即座に返すべきではないかも。
                    int n = 0;
                    int a = 0;
                    for (int p=0; p<N; p++)
                        if (D1[field.pp[p]]<oo &&
                            D3[field.pp[p]]==oo)
                        {
                            n++;
                            field.F[tp] = 1;
                            a += field.get_area(field.pp[p]);
                            field.F[tp] = 0;
                        }
                    if (n>0 &&
                        a/n <= 30)
                        return d+4;
                    //  ペットを閉じ込めてマス数が上記の条件より大きい場合は不可。
                    if (n>0 &&
                        a/n > 30)
                        continue;

                    //  (SX/2, 0)から(px, py)の最短経路上で、(px, py)からの距離が2、
                    //  もしくは、距離が3でペットの移動回数が奇数なら通行不可にする。
                    if (D1[tp]==pd-2 && D2[tp]==2 ||
                        D1[tp]==pd-3 && D2[tp]==3 && (field.pt[target]==TYPE_COW || field.pt[target]==TYPE_RABBIT))
                        moves.push_back(d+4);
                }
            }

        if (moves.empty())
        {
            if ((field.pt[target]==TYPE_COW || field.pt[target]==TYPE_RABBIT) &&
                D1[hp]==pd-2 &&
                D2[hp]==2)
                //  目標が奇数回移動で、距離2のマスにいるなら移動しない
                ;
            else
            {
                //  距離3のマスに近づく
                vector<int> SP;
                for (int p=0; p<S*S; p++)
                    if (D1[p]==pd-3 &&
                        D2[p]==3)
                        SP.push_back(p);

                if (!SP.empty())
                {
                    field.get_distances(SP, &D3);

                    for (int d=0; d<4; d++)
                        if (field.can_move(hp, d))
                        {
                            int tp = hp+dir[d];
                            if (D3[tp]==D3[hp]-1 &&
                                field.F[tp]==0)
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
        int hp = field.hp[h];

        long long score = field.score();
        int move = STAY;

        for (int d=0; d<4; d++)
        {
            int tp = hp+dir[d];
            if (field.can_block(tp))
            {
                field.F[tp] = 1;
                long long s = field.score();
                field.F[tp] = 0;
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

    vector<int> pp(N);
    for (int i=0; i<N; i++)
        pp[i] = px[i]*S+py[i];
    vector<int> hp(M);
    for (int i=0; i<M; i++)
        hp[i] = hx[i]*S+hy[i];

    Field field(pp, pt, hp);
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
        if (field.toward(field.pp[p], S/2*S+0)!=-1)
        {
            UPC[field.pt[p]]++;
            s++;
        }
    fprintf(stderr, "Uncaptured pets:  %2d (%2d, %2d, %2d, %2d, %2d)\n",
        s, UPC[0], UPC[1], UPC[2], UPC[3], UPC[4]);
    fprintf(stderr, "Current gate: %d\n", ai.current_gate);
    long long score = field.score();
    fprintf(stderr, "Score: %7.4f %%\n", score*1e-8*100);
}
