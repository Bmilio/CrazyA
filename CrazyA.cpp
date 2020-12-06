#include <bangtal>
#include <iostream>
#include <stack>
#include <vector>

#define S_GAP 50 //게임 한 칸의 간격
#define S_NUMBER 13 //게임판 세로 가로 칸 개수
#define MOBNUM 9 
#define BOSSNUM 1
#define BOSSSHIELD 8
#define PlayerSize 46

using namespace bangtal;
using namespace std;

//배경
ScenePtr scene;


//플레이어
class S_Player {

public:
    int WaterBalloonNum; //물풍선 개수
    int WaterPower; //물줄기 크기
    int Speed; //이동속도
    int NeedleNum; //바늘 개수
    bool IsonBall = false;
    bool Iswater = false;
    bool IsMob = false;
    int x = 300, y = 300; //현재 위치
    ObjectPtr z = Object::create("images/playerf.png", scene, x, y, true); //플레이어 모습

    void InitPlayer(int a, int b, int c, int d, int xx, int yy)
    {
        WaterBalloonNum = a;
        WaterPower = b;
        Speed = c + 2;
        NeedleNum = d;
        x = xx;
        y = yy;
    }
    S_Player() {
        WaterBalloonNum = 0; //물풍선 개수
        WaterPower = 0; //물줄기 크기
        Speed = 0; //이동속도
        NeedleNum = 0; //바늘 개수
    };
    ~S_Player() {};
};

//게임판

class S_Gameboard {

private:


public:
    int state = 0; // 0이면 비어있음, 1이면 물풍선이 있음, -1이면 물줄기 상태, 양수면 블록이 있음, 음수면 아이템이 있음
    int x = 0, y = 0; //현재 위치
    int item = 0;
    ObjectPtr z = Object::create("images/asd.png", scene, x, y, true); //블록 이미지

    //상태 변환 함수 ★★★★★★
    void S_statechange(int n)
    {
        switch (n)
        {
        case 3:
            state = 3;
            z->setImage("images/hardblock.png");
            break;
        case 2:
            state = 2;
            z->setImage("images/block.png");
            break;
        case 1:
            //맵에 있는 물풍선의 개수가 늘어남
            state = 1;
            z->setImage("images/ball.png");
            break;
        case 0:
            if (state == 3)
                break;

            state = item;

            if (item == 0)
                z->setImage("images/blank.png");
            else if (item == -2 || item == -3 || item == -4)
            {
                S_statechange(item);
            }

            break;
        case -1:
            if (state == 2)
            {
                item = -rand() % 6;

                if (item != -2 && item != -3 && item != -4)
                    item = 0;
            }
            else if (state == 3)
            {
                break;
            }
            state = -1;
            z->setImage("images/spli.png");
            break;
        case -2:
            state = -2;
            item = 0;
            z->setImage("images/spliitem.png");
            break;
        case -3:
            state = -3;
            item = 0;
            z->setImage("images/wateritem.png");
            break;
        case -4:
            state = -4;
            item = 0;
            z->setImage("images/shoeitem.png");
            break;


        }
    }

    //생성자같은 함수
    void S_BoardInit(int xx, int yy, int sstate)
    {

        state = sstate;
        x = xx;
        y = yy;
        z->locate(scene, x, y + 2);

        S_statechange(state);
    }

    S_Gameboard() {};
    ~S_Gameboard() {};
};

class Mob {
public:
    int speed = 5;
    int x = 1;
    int y = 1;
    int direct = rand() % 4; //(움직이는 방향)
    ObjectPtr MOB;
    void mobcreate(int a, int b) {
        MOB = Object::create("images/mobf.png", scene, a, b, false);
        x = a;
        y = b;
    }
    bool mobTouch = false;

    bool mobDead = false; // 아직 안건드린 부분

    Mob() { x = 1; y = 1; };
    ~Mob() {};

};



bool touch(int wx, int wy, int mobx, int moby) {
    return((wx <= mobx + S_GAP * 3 && wx >= mobx - S_GAP && wy <= moby + S_GAP * 3 && wy >= moby - S_GAP));
}

class BossMob : public Mob {
public:
    int speed = 5;
    int x = 1;
    int y = 1;
    int direct = 2; //(움직이는 방향)
    ObjectPtr MOB;

    void mobcreate(int a, int b) {
        MOB = Object::create("images/bossfront.png", scene, a, b, false);
        x = a;
        y = b;
    }

    int bossAttack = 0;

    bool mobStop = false;

    bool mobDead = false;

};


Mob mob[MOBNUM];
TimerPtr mobTimer[MOBNUM];
int mobDieCount;

BossMob bossMob[BOSSNUM];
TimerPtr bossTimer[BOSSNUM];
int bossDieCount;
TimerPtr mobCheckTimer[2];


//물풍선 함수
TimerPtr waterTimer;

bool IsRecCross(S_Player n, S_Gameboard qw) //플레이어와 블럭이 겹치는가?
{
    int x1 = n.x + S_GAP - PlayerSize, y1 = n.y + S_GAP - PlayerSize, x2 = n.x + PlayerSize, y2 = n.y + PlayerSize;
    int x3 = qw.x, y3 = qw.y, x4 = qw.x + S_GAP, y4 = qw.y + S_GAP;

    if (x2 <= x3)
        return false;
    if (x4 <= x1)
        return false;
    if (y4 <= y1)
        return false;
    if (y2 <= y3)
        return false;

    if (qw.state >= 2)
        return true;
    else if (qw.state == 1 && !n.IsonBall)
        return true;
    else
        return false;
}
bool IsRecCross2(S_Player n, S_Gameboard qw) //플레이어와 블럭이 겹치는가?
{
    int x1 = n.x + S_GAP - PlayerSize, y1 = n.y + S_GAP - PlayerSize, x2 = n.x + PlayerSize, y2 = n.y + PlayerSize;
    int x3 = qw.x, y3 = qw.y, x4 = qw.x + S_GAP, y4 = qw.y + S_GAP;

    if (x2 <= x3)
        return false;
    if (x4 <= x1)
        return false;
    if (y4 <= y1)
        return false;
    if (y2 <= y3)
        return false;

    return true;
}
bool IsRecCross3(S_Player n, S_Gameboard qw) //플레이어와 블럭이 겹치는가?
{
    int x1 = n.x + S_GAP - PlayerSize + 5, y1 = n.y + S_GAP - PlayerSize + 5, x2 = n.x + PlayerSize - 5, y2 = n.y + PlayerSize - 5;
    int x3 = qw.x, y3 = qw.y, x4 = qw.x + S_GAP, y4 = qw.y + S_GAP;

    if (x2 <= x3)
        return false;
    if (x4 <= x1)
        return false;
    if (y4 <= y1)
        return false;
    if (y2 <= y3)
        return false;

    return true;
}
bool IsRecCross3(S_Player n, Mob qw) //플레이어와 몹이 겹치는가?
{
    int x1 = n.x + S_GAP - PlayerSize + 5, y1 = n.y + S_GAP - PlayerSize + 5, x2 = n.x + PlayerSize - 5, y2 = n.y + PlayerSize - 5;
    int x3 = qw.x + 3, y3 = qw.y + 3, x4 = qw.x + S_GAP - 3, y4 = qw.y + S_GAP - 3;

    if (x2 <= x3)
        return false;
    if (x4 <= x1)
        return false;
    if (y4 <= y1)
        return false;
    if (y2 <= y3)
        return false;

    return true;
}
bool IsRecCross4(S_Player n, BossMob qw) //플레이어와 몹이 겹치는가?
{
    int x1 = n.x + S_GAP - PlayerSize + 5, y1 = n.y + S_GAP - PlayerSize + 5, x2 = n.x + PlayerSize - 5, y2 = n.y + PlayerSize - 5;
    int x3 = qw.x + 6, y3 = qw.y + 6, x4 = qw.x + S_GAP * 3 - 6, y4 = qw.y + S_GAP * 3 - 6;

    if (x2 <= x3)
        return false;
    if (x4 <= x1)
        return false;
    if (y4 <= y1)
        return false;
    if (y2 <= y3)
        return false;

    return true;
}

int main()
{
    scene = Scene::create("1", "images/screen.png");
    S_Gameboard Round1[S_NUMBER * S_NUMBER];

    int q3 = true;

    ObjectPtr num[7][11];

    for (int i = 0; i < 5; i++)
    {
        num[i][0] = Object::create("images/0.png", scene, 0, 0, false);
        num[i][1] = Object::create("images/1.png", scene, 0, 0, false);
        num[i][2] = Object::create("images/2.png", scene, 0, 0, false);
        num[i][3] = Object::create("images/3.png", scene, 0, 0, false);
        num[i][4] = Object::create("images/4.png", scene, 0, 0, false);
        num[i][5] = Object::create("images/5.png", scene, 0, 0, false);
        num[i][6] = Object::create("images/6.png", scene, 0, 0, false);
        num[i][7] = Object::create("images/7.png", scene, 0, 0, false);
        num[i][8] = Object::create("images/8.png", scene, 0, 0, false);
        num[i][9] = Object::create("images/9.png", scene, 0, 0, false);
    }

    for (int i = 0; i < 10; i++)
        num[0][i]->locate(scene, 880, 300); //물풍선
    for (int i = 0; i < 10; i++)
        num[2][i]->locate(scene, 880, 100); //바늘
    for (int i = 0; i < 10; i++)
        num[3][i]->locate(scene, 1160, 300); //신발
    for (int i = 0; i < 10; i++)
        num[4][i]->locate(scene, 1160, 100); //물줄기

        //1라운드 맵

    for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
    {
        Round1[i].S_BoardInit(42 + (i % S_NUMBER) * S_GAP, 720 - 66 - (i / S_NUMBER) * S_GAP, 0);
    }

    for (int i = 0; i < S_NUMBER * S_NUMBER; i++)//★★★★★★
    {
        if (13 * 3 < i && i < 13 * 10)
            if ((i % S_NUMBER) == 3 || (i % S_NUMBER) == 4 || (i % S_NUMBER) == 9 || (i % S_NUMBER) == 8)
                Round1[i].S_statechange(2); // 블럭을 설치
    }



    Round1[61].S_statechange(3);
    Round1[125].S_statechange(3);
    Round1[107].S_statechange(3);
    Round1[43].S_statechange(3);

    for (int i = 0; i < MOBNUM; i++) {             // 각 mob들 스테이지별로 랜덤위치 생성 완료

        int qq = rand() % (S_NUMBER * S_NUMBER);

        while (1)
        {
            if (Round1[qq].state == 0)
                break;
            else
                qq = (qq + 1) % (S_NUMBER * S_NUMBER);
        }

        mob[i].mobcreate(Round1[qq].x, Round1[qq].y);
        mobTimer[i] = Timer::create(2.0f);
    }


    for (int i = 0; i < BOSSNUM; i++) {
        //int qq = rand() % (S_NUMBER * S_NUMBER);

        //while (1)
        //{
        //    if (Round1[qq].state == 0)
        //        break;
        //    else
        //        qq = (qq + 1) % (S_NUMBER * S_NUMBER);
        //}
        bossMob[i].mobcreate(300, 350);
        bossTimer[i] = Timer::create(2.0f);
    }

    ObjectPtr nextButton1 = Object::create("images/next.png", scene, 100, 400, false);
    nextButton1->setScale(0.7f);

    for (int i = 0; i < 2; i++) {
        mobCheckTimer[i] = Timer::create(1.0f);
    }

    setGameOption(GameOption::GAME_OPTION_INVENTORY_BUTTON, false);
    setGameOption(GameOption::GAME_OPTION_MESSAGE_BOX_BUTTON, false);
    setGameOption(GameOption::GAME_OPTION_ROOM_TITLE, false);

    auto timer1 = Timer::create(0.01f);
    auto timer2 = Timer::create(0.01f);
    auto timer3 = Timer::create(0.01f);
    auto timer4 = Timer::create(0.01f);
    auto bossat = Timer::create(8.0f);
    auto gameover = Timer::create(8.0f);
    auto waterTimerBoss = Timer::create(1.0f);
    auto waterTimerBoss2 = Timer::create(0.2f);


    TimerPtr waterTimer[4];
    for (int i = 0; i < 4; i++)
        waterTimer[i] = Timer::create(5.0f); //5초 뒤에 바뀜

    TimerPtr waterTimer2[4];
    for (int i = 0; i < 4; i++)
        waterTimer2[i] = Timer::create(0.5f);



    bool k1 = false;
    bool k2 = false;
    bool k3 = false;
    bool k4 = false;
    bool bobo = true;

    stack<int> st[4], st1[4];
    stack<int> kk, kkk;
    int s = 0; //물줄기로 바뀔 예정인 블록 번호들 저장

    vector<int> vec[4], wvec;


    S_Player nu;
    nu.InitPlayer(1, 1, 2, 3, 300, 300);

    num[0][nu.WaterBalloonNum]->show(); //물풍선
    num[2][nu.NeedleNum]->show(); //바늘
    num[3][nu.Speed]->show(); //신발
    num[4][nu.WaterPower]->show(); //물줄기


    scene->setOnKeyboardCallback([&](ScenePtr scene, int key, bool pressed)->bool {

        int min = 0;
        int a = abs(Round1[min].x - nu.x);
        int b = abs(Round1[min].y - nu.y);

        if (q3)
            if (pressed) { //b를 누르면 몬스터가 나옴
                for (int i = 0; i < MOBNUM / 3; i++) {
                    mob[i].MOB->show();
                    mobTimer[i]->start();
                    mobCheckTimer[0]->start();
                }
                //bossMob[0].MOB->show();
                //bossTimer[0]->start();
                q3 = false;
            }

        //지금 플레이어와 가장 가까운 블록 번호(min)을 찾는다
        for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
            if ((abs(Round1[i].x - nu.x) * abs(Round1[i].x - nu.x) + abs(Round1[i].y - nu.y) * abs(Round1[i].y - nu.y) <= a * a + b * b))
            {
                a = abs(Round1[i].x - nu.x);
                b = abs(Round1[i].y - nu.y);
                min = i;
            }
        }

        if (nu.Iswater)
        {
            if (key == 12 && nu.NeedleNum > 0) {        // l을 누르면 바늘이 사용됨
                nu.NeedleNum--;
                num[2][nu.NeedleNum + 1]->hide();
                num[2][nu.NeedleNum]->show();
                nu.z->setImage("images/playerf.png");
                nu.Iswater = false;
                gameover->stop();
                gameover->set(8.0f);
            }
        }
        else {
            if (key == 1 && pressed) {        //왼쪽이동
                k1 = pressed;
                timer1->start();
            }
            else if (key == 1) {
                k1 = pressed;
                timer1->stop();
            }
            else if (key == 4 && pressed) {   //오른쪽이동
                k2 = pressed;
                timer2->start();
            }
            else if (key == 4) {
                k1 = pressed;
                timer2->stop();
            }
            else if (key == 23 && pressed) {   //위쪽이동
                k3 = pressed;
                timer3->start();
            }
            else if (key == 23) {
                k3 = pressed;
                timer3->stop();
            }
            else if (key == 19 && pressed) {   //아래쪽이동
                k4 = pressed;
                timer4->start();
            }
            else if (key == 19) {
                k4 = pressed;
                timer4->stop();
            }
            else if (key == 11 && !pressed) { //물풍선 놓기 k를 누름

                int qw = 0;

                for (int i = 0; i < S_NUMBER * S_NUMBER; i++) //맵에 있는 물풍선의 개수를 확인
                    if (Round1[i].state == 1)
                        qw++;
                if (qw >= nu.WaterBalloonNum)
                    return true;

                if (Round1[min].state != 0) //비어 있는 상태가 아니면 그냥 끝
                    return true;

                if (nu.IsonBall) //물풍선 위에 있으면 설치할 수 없음
                    return true;

                nu.IsonBall = true; // 물풍선 위에 있다.
                Round1[min].S_statechange(1); //지금 이 블록은 물풍선이 있는 상태

                vec[s].push_back(min); //우선 물풍선이 놓인 자리를 저장
                wvec.push_back(min * 10 + s); //물풍선의 자리와 s 저장

                int n = min % S_NUMBER; //x좌표
                int m = min / S_NUMBER; //y좌표

                if (0 <= n + 1 && n + 1 < S_NUMBER) //x좌표가 범위 안이면 ★★★★★★
                    for (int i = min + 1; (min / S_NUMBER) * S_NUMBER <= i && i <= (min / S_NUMBER) * S_NUMBER + S_NUMBER - 1 && i < min + 1 + nu.WaterPower; i++)
                    {
                        if (Round1[i].state == 3)
                            break;
                        vec[s].push_back(i);
                        if (Round1[i].state == 2)
                            break;

                    }
                if (0 <= n - 1 && n - 1 < S_NUMBER) //x좌표가 범위 안이면
                    for (int i = min - 1; (min / S_NUMBER) * S_NUMBER <= i && i <= (min / S_NUMBER) * S_NUMBER + S_NUMBER - 1 && i > min - 1 - nu.WaterPower; i--)
                    {
                        if (Round1[i].state == 3)
                            break;
                        vec[s].push_back(i);

                        if (Round1[i].state == 2)
                            break;

                    }
                if (0 <= m + 1 && m + 1 < S_NUMBER) //x좌표가 범위 안이면
                    for (int i = min + S_NUMBER; 0 <= i && i < S_NUMBER * S_NUMBER && i < min + S_NUMBER + nu.WaterPower * S_NUMBER; i = i + S_NUMBER)
                    {
                        if (Round1[i].state == 3)
                            break;
                        vec[s].push_back(i);

                        if (Round1[i].state == 2)
                            break;

                    }
                if (0 <= m - 1 && m - 1 < S_NUMBER) //x좌표가 범위 안이면
                    for (int i = min - S_NUMBER; 0 <= i && i<S_NUMBER * S_NUMBER && i > min - S_NUMBER - nu.WaterPower * S_NUMBER; i = i - S_NUMBER)
                    {
                        if (Round1[i].state == 3)
                            break;
                        vec[s].push_back(i);

                        if (Round1[i].state == 2)
                            break;

                    }

                waterTimer[s]->start();

                s = (++s) % 4;

                cout << "S=" << s << endl;
            }
        }

        return true;
        });


    waterTimer[0]->setOnTimerCallback([&](TimerPtr t)->bool {

        int p = 0;

        cout << 1 << endl;

        for (auto i : vec[p])
        {
            cout << 1 << endl;
            for (auto j : wvec)
            {
                cout << 1 << endl;
                int kl = j / 10; //min의 값
                int kl2 = j % 10; //s의 값

                if (kl2 != p && i == kl) //자기 자신 아닌곳에서 물풍선이 있다면 터트린다.
                {
                    cout << 111 << endl;
                    waterTimer[kl2]->stop();
                    waterTimer[kl2]->set(0.001f);
                    waterTimer[kl2]->start();
                }
            }

            st[p].push(i);
            st1[p].push(i);
        }

        for (auto iter = wvec.begin(); iter != wvec.end(); iter++)
        {
            cout << 2 << endl;
            if (*(iter) % 10 == p)
            {
                wvec.erase(iter);
                break;

            }
        }

        while (!st[0].empty())
        {
            if (IsRecCross3(nu, Round1[st[0].top()])) //물줄기와 만나면
            {
                nu.Iswater = true;
                nu.z->setImage("images/watered.png");
                gameover->start();
            }

            Round1[st[0].top()].S_statechange(-1);
            st[0].pop();
        }


        waterTimer[0]->set(5.0f);

        waterTimer2[0]->start();

        return true;
        });

    waterTimer2[0]->setOnTimerCallback([&](TimerPtr t)->bool {

        while (!st1[0].empty())
        {
            Round1[st1[0].top()].S_statechange(0);
            st1[0].pop();
        }

        waterTimer2[0]->set(0.5f);

        vec[0].clear();


        return true;
        });

    waterTimer[1]->setOnTimerCallback([&](TimerPtr t)->bool {
        int p = 1;

        for (auto i : vec[p])
        {
            for (auto j : wvec)
            {
                int kl = j / 10; //min의 값
                int kl2 = j % 10; //s의 값

                if (kl2 != p && i == kl) //자기 자신 아닌곳에서 물풍선이 있다면 터트린다.
                {
                    waterTimer[kl2]->stop();
                    waterTimer[kl2]->set(0.001f);
                    waterTimer[kl2]->start();
                }
            }

            st[p].push(i);
            st1[p].push(i);
        }

        for (auto iter = wvec.begin(); iter != wvec.end(); iter++)
        {
            if (*(iter) % 10 == p)
            {
                wvec.erase(iter);
                break;
            }
        }
        while (!st[1].empty())
        {
            if (IsRecCross3(nu, Round1[st[1].top()])) //물줄기와 만나면
            {
                nu.Iswater = true;
                nu.z->setImage("images/watered.png");
                gameover->start();
            }
            Round1[st[1].top()].S_statechange(-1);
            st[1].pop();
        }


        waterTimer[1]->set(5.0f);

        waterTimer2[1]->start();

        return true;
        });

    waterTimer2[1]->setOnTimerCallback([&](TimerPtr t)->bool {

        while (!st1[1].empty())
        {
            Round1[st1[1].top()].S_statechange(0);
            st1[1].pop();
        }

        waterTimer2[1]->set(0.5f);

        vec[1].clear();


        return true;
        });

    waterTimer[2]->setOnTimerCallback([&](TimerPtr t)->bool {
        int p = 2;

        for (auto i : vec[p])
        {
            for (auto j : wvec)
            {
                int kl = j / 10; //min의 값
                int kl2 = j % 10; //s의 값

                if (kl2 != p && i == kl) //자기 자신 아닌곳에서 물풍선이 있다면 터트린다.
                {
                    waterTimer[kl2]->stop();
                    waterTimer[kl2]->set(0.001f);
                    waterTimer[kl2]->start();
                }
            }

            st[p].push(i);
            st1[p].push(i);
        }

        for (auto iter = wvec.begin(); iter != wvec.end(); iter++)
        {
            if (*(iter) % 10 == p)
            {
                wvec.erase(iter);
                break;
            }
        }
        while (!st[2].empty())
        {
            if (IsRecCross3(nu, Round1[st[2].top()])) //물줄기와 만나면
            {
                nu.Iswater = true;
                nu.z->setImage("images/watered.png");
                gameover->start();
            }
            Round1[st[2].top()].S_statechange(-1);
            st[2].pop();
        }


        waterTimer[2]->set(5.0f);

        waterTimer2[2]->start();

        return true;
        });

    waterTimer2[2]->setOnTimerCallback([&](TimerPtr t)->bool {

        while (!st1[2].empty())
        {
            Round1[st1[2].top()].S_statechange(0);
            st1[2].pop();
        }

        waterTimer2[2]->set(0.5f);

        vec[2].clear();


        return true;
        });

    waterTimer[3]->setOnTimerCallback([&](TimerPtr t)->bool {
        int p = 3;

        for (auto i : vec[p])
        {
            for (auto j : wvec)
            {
                int kl = j / 10; //min의 값
                int kl2 = j % 10; //s의 값

                if (kl2 != p && i == kl) //자기 자신 아닌곳에서 물풍선이 있다면 터트린다.
                {
                    waterTimer[kl2]->stop();
                    waterTimer[kl2]->set(0.001f);
                    waterTimer[kl2]->start();
                }
            }

            st[p].push(i);
            st1[p].push(i);
        }

        for (auto iter = wvec.begin(); iter != wvec.end(); iter++)
        {
            if (*(iter) % 10 == p)
            {
                wvec.erase(iter);
                break;
            }
        }
        while (!st[3].empty())
        {
            if (IsRecCross3(nu, Round1[st[3].top()])) //물줄기와 만나면
            {
                nu.Iswater = true;
                nu.z->setImage("images/watered.png");
                gameover->start();
            }
            Round1[st[3].top()].S_statechange(-1);
            st[3].pop();
        }


        waterTimer[3]->set(5.0f);

        waterTimer2[3]->start();

        return true;
        });

    waterTimer2[3]->setOnTimerCallback([&](TimerPtr t)->bool {

        while (!st1[3].empty())
        {
            Round1[st1[3].top()].S_statechange(0);
            st1[3].pop();
        }

        waterTimer2[3]->set(0.5f);

        vec[3].clear();


        return true;
        });


    timer1->setOnTimerCallback([&](TimerPtr t)->bool {



        if (nu.Iswater)
            return true;
        nu.z->setImage("images/playerl.png");


        nu.x -= nu.Speed;

        if (nu.IsonBall)
        {
            int asd = false;

            for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
            {
                if (IsRecCross2(nu, Round1[i]) && Round1[i].state == 1)
                {
                    asd = true;
                }
            }

            nu.IsonBall = asd;
        }

        for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
        {
            if (IsRecCross(nu, Round1[i]))
            {
                nu.x = Round1[i].x + S_GAP;
                break;
            }
        }

        for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
        {
            int ite = Round1[i].state;

            if (IsRecCross3(nu, Round1[i])) //만났는데 아이템이 있다면
            {
                cout << "Here " << ite << endl;

                if (ite == -2)
                {
                    if (nu.WaterPower < 7)
                        nu.WaterPower++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[4][nu.WaterPower - 1]->hide(); //물줄기
                    num[4][nu.WaterPower]->show(); //물줄기
                }
                else if (ite == -3)
                {
                    if (nu.WaterBalloonNum < 4)
                        nu.WaterBalloonNum++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[0][nu.WaterBalloonNum - 1]->hide(); //물풍선
                    num[0][nu.WaterBalloonNum]->show(); //물풍선
                }
                else if (ite == -4)
                {
                    if (nu.Speed < 8)
                        nu.Speed++;

                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[3][nu.Speed - 1]->hide(); //신발

                    num[3][nu.Speed]->show(); //신발
                }

                break;
            }

        }

        if (nu.x < Round1[0].x)
        {
            nu.x = Round1[0].x;
        }


        nu.z->locate(scene, nu.x, nu.y + 5);
        for (int i = 0; i < 1; i++) {
            t->set(0.03f);
            t->start();
        }


        return true;
        });

    if (k1)
        timer1->start();

    timer2->setOnTimerCallback([&](TimerPtr t)->bool {



        if (nu.Iswater)
            return true;
        nu.z->setImage("images/playerr.png");

        nu.x += nu.Speed;

        if (nu.IsonBall)
        {
            int asd = false;

            for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
            {
                if (IsRecCross2(nu, Round1[i]) && Round1[i].state == 1)
                {
                    asd = true;
                }
            }

            nu.IsonBall = asd;
        }

        for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
        {
            if (IsRecCross(nu, Round1[i]))
            {
                nu.x = Round1[i].x - S_GAP;
                break;
            }
        }
        for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
        {
            int ite = Round1[i].state;

            if (IsRecCross3(nu, Round1[i])) //만났는데 아이템이 있다면
            {
                cout << "Here " << ite << endl;

                if (ite == -2)
                {
                    if (nu.WaterPower < 7)
                        nu.WaterPower++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[4][nu.WaterPower - 1]->hide(); //물줄기
                    num[4][nu.WaterPower]->show(); //물줄기
                }
                else if (ite == -3)
                {
                    if (nu.WaterBalloonNum < 4)
                        nu.WaterBalloonNum++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[0][nu.WaterBalloonNum - 1]->hide(); //물풍선
                    num[0][nu.WaterBalloonNum]->show(); //물풍선
                }
                else if (ite == -4)
                {
                    if (nu.Speed < 8)
                        nu.Speed++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[3][nu.Speed - 1]->hide(); //신발
                    num[3][nu.Speed]->show(); //신발
                }
                break;
            }
        }

        if (nu.x > Round1[12].x)
        {
            nu.x = Round1[12].x;
        }


        nu.z->locate(scene, nu.x, nu.y + 5);

        for (int i = 0; i < 1; i++) {
            t->set(0.03f);
            t->start();
        }
        return true;
        });

    if (k2)
        timer2->start();

    timer3->setOnTimerCallback([&](TimerPtr t)->bool {



        if (nu.Iswater)
            return true;
        nu.z->setImage("images/playerb.png");

        nu.y += nu.Speed;

        if (nu.IsonBall)
        {
            int asd = false;

            for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
            {
                if (IsRecCross2(nu, Round1[i]) && Round1[i].state == 1)
                {
                    asd = true;
                }
            }

            nu.IsonBall = asd;
        }

        for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
        {
            if (IsRecCross(nu, Round1[i]))
            {
                nu.y = Round1[i].y - S_GAP;
                break;
            }
        }
        for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
        {
            int ite = Round1[i].state;

            if (IsRecCross3(nu, Round1[i])) //만났는데 아이템이 있다면
            {
                cout << "Here " << ite << endl;

                if (ite == -2)
                {
                    if (nu.WaterPower < 7)
                        nu.WaterPower++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[4][nu.WaterPower - 1]->hide(); //물줄기
                    num[4][nu.WaterPower]->show(); //물줄기
                }
                else if (ite == -3)
                {
                    if (nu.WaterBalloonNum < 4)
                        nu.WaterBalloonNum++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[0][nu.WaterBalloonNum - 1]->hide(); //물풍선
                    num[0][nu.WaterBalloonNum]->show(); //물풍선
                }
                else if (ite == -4)
                {
                    if (nu.Speed < 8)
                        nu.Speed++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[3][nu.Speed - 1]->hide(); //신발
                    num[3][nu.Speed]->show(); //신발
                }
                break;
            }
        }

        if (nu.y > Round1[0].y)
        {
            nu.y = Round1[0].y;
        }

        nu.z->locate(scene, nu.x, nu.y + 5);

        for (int i = 0; i < 1; i++) {
            t->set(0.03f);
            t->start();
        }
        return true;
        });

    if (k3)
        timer3->start();

    timer4->setOnTimerCallback([&](TimerPtr t)->bool {



        if (nu.Iswater)
            return true;
        nu.z->setImage("images/playerf.png");

        nu.y -= nu.Speed;

        if (nu.IsonBall)
        {
            int asd = false;

            for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
            {
                if (IsRecCross2(nu, Round1[i]) && Round1[i].state == 1)
                {
                    asd = true;
                }
            }

            nu.IsonBall = asd;
        }

        for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
        {
            if (IsRecCross(nu, Round1[i]))
            {
                nu.y = Round1[i].y + S_GAP;
                break;
            }
        }
        for (int i = 0; i < S_NUMBER * S_NUMBER; i++)
        {
            int ite = Round1[i].state;

            if (IsRecCross3(nu, Round1[i])) //만났는데 아이템이 있다면
            {
                cout << "Here " << ite << endl;

                if (ite == -2)
                {
                    if (nu.WaterPower < 7)
                        nu.WaterPower++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[4][nu.WaterPower - 1]->hide(); //물줄기
                    num[4][nu.WaterPower]->show(); //물줄기
                }
                else if (ite == -3)
                {
                    if (nu.WaterBalloonNum < 4)
                        nu.WaterBalloonNum++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[0][nu.WaterBalloonNum - 1]->hide(); //물풍선
                    num[0][nu.WaterBalloonNum]->show(); //물풍선
                }
                else if (ite == -4)
                {
                    if (nu.Speed < 8)
                        nu.Speed++;
                    Round1[i].item = 0;
                    Round1[i].S_statechange(0);
                    num[3][nu.Speed - 1]->hide(); //신발
                    num[3][nu.Speed]->show(); //신발
                }
                break;
            }
        }
        if (nu.y < Round1[168].y)
        {
            nu.y = Round1[168].y;
        }

        nu.z->locate(scene, nu.x, nu.y + 5);

        for (int i = 0; i < 1; i++) {
            t->set(0.03f);
            t->start();
        }
        return true;
        });

    if (k4)
        timer4->start();

    nextButton1->setOnMouseCallback([&](ObjectPtr object, int, int, MouseAction action)-> bool {
        nextButton1->hide();

        bossTimer[0]->start();
        bossMob[0].MOB->show();
        bossat->start();

        return true;
        });

    for (int i = 0; i < MOBNUM; i++) {
        mobTimer[i]->setOnTimerCallback([&](TimerPtr t)->bool {
            int j;
            for (j = 0; j < MOBNUM; j++) {
                if (mobTimer[j] == t)
                    break;
            }

            if (IsRecCross3(nu, mob[j])) {
                nu.Iswater = true;
                gameover->set(0.5f);
                gameover->start();
                //gameover->set(8.0f);
            }

            switch (mob[j].direct) {
            case 0: {
                mob[j].MOB->setImage("images/mobr.png");
                mob[j].x += mob[j].speed;    // 첫 스테이지 몹 속도 10
                if (mob[j].x >= 42 + 12 * S_GAP) mob[j].direct = 1;
                for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
                    if ((mob[j].x < Round1[i].x) && (mob[j].x + S_GAP >= Round1[i].x) && (mob[j].y == Round1[i].y) && (Round1[i].state == 1 || Round1[i].state == 2 || Round1[i].state == 3)) {
                        mob[j].direct = 1;
                    }
                    if ((mob[j].x == Round1[i].x) && (mob[j].y == Round1[i].y) && Round1[i].state == -1) {
                        mob[j].MOB->hide();
                        mob[j].mobDead = true;


                        if (j < MOBNUM / 3) {
                            mob[j + MOBNUM / 3].MOB->setImage("images/asd.png");
                            mob[j + MOBNUM / 3].MOB->show();
                            mobTimer[j + MOBNUM / 3]->start();
                            cout << j << endl;

                        }
                        mobTimer[j]->stop();
                    }
                }
                break;
            }

            case 1: {
                mob[j].MOB->setImage("images/mobl.png");

                mob[j].x -= mob[j].speed;
                if (mob[j].x <= 52) mob[j].direct = 0;
                for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
                    if ((mob[j].x > Round1[i].x) && (mob[j].x - S_GAP <= Round1[i].x) && (mob[j].y == Round1[i].y) && (Round1[i].state == 1 || Round1[i].state == 2 || Round1[i].state == 3)) {
                        mob[j].direct = 0;
                    }
                    if ((mob[j].x == Round1[i].x) && (mob[j].y == Round1[i].y) && Round1[i].state == -1) {
                        mob[j].MOB->hide();
                        mob[j].mobDead = true;

                        if (j < MOBNUM / 3) {
                            mob[j + MOBNUM / 3].MOB->setImage("images/asd.png");
                            mob[j + MOBNUM / 3].MOB->show();
                            mobTimer[j + MOBNUM / 3]->start();
                            cout << j << endl;

                        }
                        mobTimer[j]->stop();
                    }
                }
                break;
            }

            case 2: {
                mob[j].MOB->setImage("images/mobb.png");

                mob[j].y += mob[j].speed;
                if (mob[j].y >= 720 - 66) mob[j].direct = 3;
                for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
                    if ((mob[j].x == Round1[i].x) && (mob[j].y < Round1[i].y) && (mob[j].y + S_GAP >= Round1[i].y) && (Round1[i].state == 1 || Round1[i].state == 2 || Round1[i].state == 3)) {
                        mob[j].direct = 3;
                    }
                    if ((mob[j].x == Round1[i].x) && (mob[j].y == Round1[i].y) && Round1[i].state == -1) {

                        mob[j].MOB->hide();
                        mob[j].mobDead = true;
                        if (j < MOBNUM / 3) {
                            mob[j + MOBNUM / 3].MOB->setImage("images/asd.png");
                            mob[j + MOBNUM / 3].MOB->show();
                            mobTimer[j + MOBNUM / 3]->start();
                            cout << j << endl;

                        }
                        mobTimer[j]->stop();
                    }
                }
                break;

            }

            case 3: {
                mob[j].MOB->setImage("images/mobf.png");

                mob[j].y -= mob[j].speed;
                if (mob[j].y <= 720 - 66 - 12 * S_GAP) mob[j].direct = 2;
                for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
                    if ((mob[j].x == Round1[i].x) && (mob[j].y > Round1[i].y) && (mob[j].y - S_GAP <= Round1[i].y) && (Round1[i].state == 1 || Round1[i].state == 2 || Round1[i].state == 3)) {
                        mob[j].direct = 2;
                    }
                    if ((mob[j].x == Round1[i].x) && (mob[j].y == Round1[i].y) && Round1[i].state == -1) {
                        mob[j].MOB->hide();
                        mob[j].mobDead = true;

                        if (j < MOBNUM / 3) {
                            mob[j + MOBNUM / 3].MOB->setImage("images/asd.png");
                            mob[j + MOBNUM / 3].MOB->show();
                            mobTimer[j + MOBNUM / 3]->start();
                            cout << j << endl;

                        }
                        mobTimer[j]->stop();

                    }
                }
                break;
            }
            }
            mob[j].MOB->locate(scene, mob[j].x, mob[j].y);

            for (int i = 0; i < 30; i++) {
                t->set(0.05f);
                t->start();

                if (mob[j].mobDead == true) t->stop();
            }
            return true;
            });
    }

    TimerPtr mobStopTimer = Timer::create(1.0f);    //********************** 수정부분(보스 맞을때마다 순간 사라지는 타이머

    mobStopTimer->setOnTimerCallback([&](TimerPtr t)->bool {   //********************** 수정부분(보스 맞을때마다 순간 사라지는 타이머



        if (bossMob[0].bossAttack == BOSSSHIELD) bossMob[0].MOB->hide();
        else {
            bossMob[0].mobStop = false;
            bossMob[0].MOB->show();
            mobStopTimer->set(1.0f);
        }

        return true;
        });

    for (int i = 0; i < BOSSNUM; i++) {                  //********************** 수정부분
        bossTimer[i]->setOnTimerCallback([&](TimerPtr t)->bool {
            int j;
            for (j = 0; j < BOSSNUM; j++) {
                if (bossTimer[j] == t)
                    break;
            }

            if (IsRecCross4(nu, bossMob[j])) {
                nu.Iswater = true;
                gameover->set(0.5f);
                gameover->start();
            }

            // 0 : 오른쪽, 1 왼쪽 2 위쪽 3 아래쪽
            switch (bossMob[j].direct) {
            case 0: {
                bossMob[j].MOB->setImage("images/bossright.png");

                bossMob[j].x += bossMob[j].speed;
                if (bossMob[j].x + S_GAP * 3 >= 42 + 13 * S_GAP && bossMob[j].y < 350) bossMob[j].direct = 2;  //오른쪽 벽 부딪히면 위로
                if (bossMob[j].x + S_GAP * 3 >= 42 + 13 * S_GAP && bossMob[j].y > 350) bossMob[j].direct = 3;
                for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
                    if (touch(Round1[i].x, Round1[i].y, bossMob[j].x, bossMob[j].y) && (Round1[i].state == 1 || Round1[i].state == 2 || Round1[i].state == 3)) {
                        bossMob[j].direct = 1;
                    }
                    if (touch(Round1[i].x, Round1[i].y, bossMob[j].x, bossMob[j].y) && Round1[i].state == -1) {
                        if (bossMob[j].mobStop == true) {
                            bossMob[j].MOB->hide();
                            mobStopTimer->start();
                        }
                        else {
                            bossMob[j].bossAttack++;
                            cout << bossMob[j].bossAttack << endl;
                            bossMob[j].direct = 1;
                            bossMob[j].mobStop = true;
                        }
                    }
                }
                break;
            }
            case 1: {
                bossMob[j].MOB->setImage("images/bossleft.png");

                bossMob[j].x -= bossMob[j].speed;
                if (bossMob[j].x <= 42 && bossMob[j].y > 350) bossMob[j].direct = 3;
                if (bossMob[j].x <= 42 && bossMob[j].y < 350) bossMob[j].direct = 2;
                for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
                    if (touch(Round1[i].x, Round1[i].y, bossMob[j].x, bossMob[j].y) && (Round1[i].state == 1 || Round1[i].state == 2 || Round1[i].state == 3)) {
                        bossMob[j].direct = 0;
                    }
                    if (touch(Round1[i].x, Round1[i].y, bossMob[j].x, bossMob[j].y) && Round1[i].state == -1) {
                        if (bossMob[j].mobStop == true) {
                            bossMob[j].MOB->hide();
                            mobStopTimer->start();
                        }
                        else {
                            bossMob[j].bossAttack++;
                            cout << bossMob[j].bossAttack << endl;
                            bossMob[j].direct = 0;
                            bossMob[j].mobStop = true;
                        }
                    }
                }
                break;
            }
            case 2: {
                bossMob[j].MOB->setImage("images/bossback.png");

                bossMob[j].y += bossMob[j].speed;
                if (bossMob[j].y + S_GAP * 2 >= 720 - 66 && bossMob[j].x < 200) bossMob[j].direct = 0;
                if (bossMob[j].y + S_GAP * 2 >= 720 - 66 && bossMob[j].x > 200) bossMob[j].direct = 1;
                for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
                    if (touch(Round1[i].x, Round1[i].y, bossMob[j].x, bossMob[j].y) && (Round1[i].state == 1 || Round1[i].state == 2 || Round1[i].state == 3)) {
                        bossMob[j].direct = 3;
                    }
                    if (touch(Round1[i].x, Round1[i].y, bossMob[j].x, bossMob[j].y) && Round1[i].state == -1) {
                        if (bossMob[j].mobStop == true) {
                            bossMob[j].MOB->hide();
                            mobStopTimer->start();
                        }
                        else {
                            bossMob[j].bossAttack++;
                            cout << bossMob[j].bossAttack << endl;
                            bossMob[j].direct = 3;
                            bossMob[j].mobStop = true;
                        }
                    }
                }
                break;

            }
            case 3: {
                bossMob[j].MOB->setImage("images/bossfront.png");

                bossMob[j].y -= bossMob[j].speed;
                if (bossMob[j].y <= 720 - 66 - 12 * S_GAP && bossMob[j].x > 200) bossMob[j].direct = 1;
                if (bossMob[j].y <= 720 - 66 - 12 * S_GAP && bossMob[j].x < 200) bossMob[j].direct = 0;
                for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
                    if (touch(Round1[i].x, Round1[i].y, bossMob[j].x, bossMob[j].y) && (Round1[i].state == 1 || Round1[i].state == 2 || Round1[i].state == 3)) {
                        bossMob[j].direct = 2;
                        cout << "checked!" << endl;
                    }
                    if (touch(Round1[i].x, Round1[i].y, bossMob[j].x, bossMob[j].y) && Round1[i].state == -1) {
                        if (bossMob[j].mobStop == true) {
                            bossMob[j].MOB->hide();
                            mobStopTimer->start();
                        }
                        else {
                            bossMob[j].bossAttack++;
                            cout << bossMob[j].bossAttack << endl;
                            bossMob[j].direct = 2;
                            bossMob[j].mobStop = true;
                        }
                    }
                }
                break;
            }
            }

            bossMob[j].MOB->locate(scene, bossMob[j].x, bossMob[j].y);

            for (int i = 0; i < 100; i++) {
                t->set(0.05f);
                t->start();

                if (bossMob[j].bossAttack == BOSSSHIELD)
                {
                    if (bobo)
                    {
                        bossMob[j].bossAttack = -3;
                        bobo = false;
                        for (int i = MOBNUM / 3 * 2; i < MOBNUM; i++) {
                            mob[i].MOB->show();
                            mobTimer[i]->start();
                        }
                        mobCheckTimer[1]->start();
                    }
                    else
                    {
                        bossMob[j].MOB->hide();
                        bossat->stop();
                        auto win = Object::create("images/win.png", scene, 120, 500, true);
                        win->setScale(0.7f);
                        nu.Iswater = true;
                        t->stop();

                    }
                }

            }
            return true;
            });
    }


    mobCheckTimer[0]->setOnTimerCallback([&](TimerPtr t)->bool {
        for (int i = 0; i < MOBNUM / 3 * 2; i++) {
            if (mob[i].mobDead) mobDieCount++;
        }
        if (mobDieCount == MOBNUM / 3 * 2) {
            nextButton1->show();
            t->stop();
            mobDieCount = 0;
        }
        else {
            mobDieCount = 0;
            t->set(0.1f);
            t->start();
        }

        cout << "mobDieCOunt : " << mobDieCount << endl;
        return true;
        });

    mobCheckTimer[1]->setOnTimerCallback([&](TimerPtr t)->bool {   //********************** 수정부분(보스 이후에 나오는 작은 몹들 죽었는지 1초마다 카운트하는거)
        for (int i = MOBNUM / 3 * 2; i < MOBNUM; i++) {
            if (mob[i].mobDead)
                mobDieCount++;
        }
        if (mobDieCount == MOBNUM / 3) {

            t->stop();
        }
        else {
            mobDieCount = 0;
            t->set(0.1f);
            t->start();
        }

        cout << "mobDieCOunt : " << mobDieCount << endl;
        return true;
        });


    bossat->setOnTimerCallback([&](TimerPtr t)->bool {


        int min, a, b;
        a = abs(Round1[0].x - bossMob[0].x);
        b = abs(Round1[0].y - bossMob[0].y);

        for (int i = 0; i < S_NUMBER * S_NUMBER; i++) {
            if ((abs(Round1[i].x - bossMob[0].x) * abs(Round1[i].x - bossMob[0].x) + abs(Round1[i].y - bossMob[0].y) * abs(Round1[i].y - bossMob[0].y) <= a * a + b * b))
            {
                a = abs(Round1[i].x - bossMob[0].x);
                b = abs(Round1[i].y - bossMob[0].y);
                min = i;
            }
        }
        int pmax = 8;

        int b1x, b1y;

        for (int i = 0; i < pmax + 1; i++)
        {
            b1x = Round1[min].x - (3 - i) * S_GAP, b1y = Round1[min].y + 5 * S_GAP;

            if (Round1[0].x <= b1x && b1x <= Round1[168].x && Round1[168].y <= b1y && b1y <= Round1[0].y)
                kk.push(min - (3 - i) - 5 * S_NUMBER);
        }

        for (int i = 0; i < pmax + 1; i++)
        {
            b1x = Round1[min].x - (3 - i) * S_GAP, b1y = Round1[min].y - 3 * S_GAP;

            if (Round1[0].x <= b1x && b1x <= Round1[168].x && Round1[168].y <= b1y && b1y <= Round1[0].y)
                kk.push(min - (3 - i) + 3 * S_NUMBER);
        }

        for (int i = 0; i < pmax - 1; i++)
        {
            b1x = Round1[min].x - 3 * S_GAP, b1y = Round1[min].y - (2 - i) * S_GAP;

            if (Round1[0].x <= b1x && b1x <= Round1[168].x && Round1[168].y <= b1y && b1y <= Round1[0].y)
                kk.push(min - 3 + (2 - i) * S_NUMBER);
        }

        for (int i = 0; i < pmax - 1; i++)
        {
            b1x = Round1[min].x + 5 * S_GAP, b1y = Round1[min].y - (2 - i) * S_GAP;

            if (Round1[0].x <= b1x && b1x <= Round1[168].x && Round1[168].y <= b1y && b1y <= Round1[0].y)
                kk.push(min + 5 + (2 - i) * S_NUMBER);
        }

        switch (bossMob[0].direct)
        {
        case 0:
            bossMob[0].MOB->setImage("images/br.png");
            break;
        case 1:
            bossMob[0].MOB->setImage("images/bl.png");
            break;
        case 2:
            bossMob[0].MOB->setImage("images/bb.png");
            break;
        case 3:
            bossMob[0].MOB->setImage("images/bf.png");
            break;
        }

        waterTimerBoss->start();

        bossat->set(8.0f);

        bossat->start();

        return true;
        });

    waterTimerBoss->setOnTimerCallback([&](TimerPtr t)->bool {

        switch (bossMob[0].direct)
        {
        case 0:
            bossMob[0].MOB->setImage("images/bossright.png");
            break;
        case 1:
            bossMob[0].MOB->setImage("images/bossleft.png");
            break;
        case 2:
            bossMob[0].MOB->setImage("images/bossback.png");
            break;
        case 3:
            bossMob[0].MOB->setImage("images/bossfront.png");
            break;
        }

        while (!kk.empty())
        {
            if (IsRecCross3(nu, Round1[kk.top()])) //물줄기와 만나면
            {
                nu.Iswater = true;
                nu.z->setImage("images/watered.png");
                gameover->start();
            }

            for (auto j : wvec)
            {
                int kl = j / 10; //min의 값
                int kl2 = j % 10; //s의 값

                if (kk.top() == kl) //물풍선과 만난다면
                {
                    if (kl2 < 4)
                    {
                        waterTimer[kl2]->stop();
                        waterTimer[kl2]->set(0.001f);
                        waterTimer[kl2]->start();
                    }

                }
            }

            Round1[kk.top()].S_statechange(-1);
            kkk.push(kk.top());
            kk.pop();
        }

        waterTimerBoss2->start();
        waterTimerBoss->set(1.0f);

        return true;
        });

    waterTimerBoss2->setOnTimerCallback([&](TimerPtr t)->bool {

        while (!kkk.empty())
        {
            Round1[kkk.top()].S_statechange(0);
            kkk.pop();
        }

        waterTimerBoss2->set(0.2f);

        return true;
        });

    gameover->setOnTimerCallback([&](TimerPtr t)->bool {

        if (nu.Iswater)
        {
            auto gameover = Object::create("images/gameover.png", scene, 100, 500, true);
            gameover->setScale(0.7f);
            nu.z->setImage("images/death.png");
            nu.Iswater = true;
        }


        return true;
        });



    startGame(scene);

}