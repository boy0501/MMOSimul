#include <iostream>
#include <string>

#include <unordered_map>
#include <bitset>
#include <queue>
#include <random>
#include <iomanip>
#include "../Game/Network/Network.h"

using namespace std;

class AStar {
public:
    float F = 0.0f;      //G + H  - 최종적으로 비교해줄 값
    float G = 0.0f;      //다음 노드까지의 값
    float H = 0.0f;      //도착지점까지의 휴리스틱 값
    int x, y;
    AStar* Parent;
    AStar(int tx, int ty) :
        x(tx),
        y(ty)
    {};
    AStar() {  };
    ~AStar() {
    }
    void HCalCul(int Endx, int Endy) {
        H = abs(Endx - x) + abs(Endy - y);
        F = G + H;
    }
    void GCalCul(int Nextx, int Nexty) {
        G = sqrt(pow(Nextx - x, 2) + pow(Nexty - y, 2));
        F = G + H;
    }

};
struct cmp {
    bool operator()(AStar* n1, AStar* n2) {
        return n1->F > n2->F;
    }
};
random_device rd;
mt19937 rng(rd());
uniform_int_distribution<int> Rint(0, 49);
int map[50][50];

#define WALLNUM 900 
#define ROAD 0
#define WALL 1
#define START 2 
#define END 3

class Pos {
public:
    int x, y;
    Pos() { x = 0, y = 0; }
    Pos(int tx, int ty) :
        x(tx),
        y(ty)
    {

    }
};

Pos dir[8] = {
   Pos(-1,-1),Pos(0,-1),Pos(1,-1),
   Pos(-1,0),Pos(1,0),
   Pos(-1,1),Pos(0,1),Pos(1,1)
};
Pos dirX[4] = {
   Pos(0,-1),
   Pos(-1,0),Pos(1,0),
   Pos(0,1)
};
Pos EndPoint;
Pos StartPoint;
void Init(int myX,int myY,int targetx,int targety)
{

    for (int i = 0; i < 50; ++i)
    {
        for (int j = 0; j < 50; ++j)
        {
            map[i][j] = ROAD;
        }
    }
    for (int i = 0; i < WALLNUM; ++i)
    {
        map[Rint(rng)][Rint(rng)] = WALL;
    }
    StartPoint.x = myX;
    StartPoint.y = myY;
    EndPoint.x = targetx;
    EndPoint.y = targety;
    map[EndPoint.y][EndPoint.x] = END;
    map[0][0] = START;
}

void color(unsigned short color)
{
    HANDLE hCon = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(hCon, color);
}
void ShowResult()
{
    for (int i = 0; i < 50; ++i)
    {
        for (int j = 0; j < 50; ++j)
        {
            if (map[i][j] == 9)
                color(12);
            else if (map[i][j] == 1)
                color(11);
            else if (map[i][j] == 8)
                color(10);
            else
                color(15);
            cout << setw(2) << map[i][j];
        }
        cout << endl;
    }
}

void AStarAlgo(int myX,int myY,int targetX,int targetY,int& resultX,int& resultY)
{
    vector<AStar*> Closed;
    priority_queue<AStar*, vector<AStar*>, cmp> Open;
    bool findFail = false;

    //visit이 오버헤드가 엄청 크긴 함 .
    bool** visit = new bool*[2000];
    for (int i = 0; i < 2000; ++i)
    {
        visit[i] = new bool[2000];
        memset(visit[i], 0, sizeof(bool) * 2000);
    }
    Open.push(new AStar(myX, myY));
    visit[myX][myY] = true;

    int count = 0;
    while (!Open.empty())
    {
        auto p = Open.top();
        Open.pop();
        count++;
        if (count > 1000)
        {
            while (!Open.empty())
            {
                auto delp = Open.top();
                Open.pop();
                delete delp;
            }
            //cout << "Astar Fail." << Closed.size() << endl;
            findFail = true;
           // cout << "Astar Fail. 도착지가 너무 멉니다." << endl;
            break;
        }
        if (!Closed.empty())
        {
            if (Closed.back()->x == targetX && Closed.back()->y == targetY)
            {
                while (!Open.empty())
                {
                    auto delp = Open.top();
                    Open.pop();
                    delete delp;
                }
                break;
            }
        }

        for (auto d : dir)
        {
            if (p->x + d.x >= 0 && p->x + d.x < WORLD_WIDTH)
            {
                if (p->y + d.y >= 0 && p->y + d.y < WORLD_HEIGHT)
                {
                    if (mMap[p->x + d.x][p->y + d.y] == ROAD || (((p->x + d.x) == targetX) && ((p->y + d.y) == targetY)))
                    {
                        if (visit[p->x + d.x][p->y + d.y] == false)
                        {
                            auto tmp2 = new AStar(p->x + d.x, p->y + d.y);
                            tmp2->HCalCul(targetX, targetY);
                            tmp2->GCalCul(p->x, p->y);
                            tmp2->Parent = p;
                            visit[p->x + d.x][p->y + d.y] = true;
                            Open.push(tmp2);
                        }
                    }
                }
            }
        }
        Closed.push_back(p);
    }

	if (findFail == true)
	{

		resultX = 0;
		resultY = 0;


		for (auto p : Closed)
		{
			delete p;
		}
		for (int i = 0; i < 2000; ++i)
		{
			delete[] visit[i];
		}

		delete[] visit;
        return;
	}


    AStar* p = Closed.back();
    int i = 0;
    while (1)
    {
        if (p->Parent == nullptr)
        {
            findFail = true;
            break;
        }

        if ((p->Parent->x == myX) && (p->Parent->y == myY))
            break;
        p = p->Parent;

        i++;
        if (i > 1000)
        {
            findFail = true;
            break;
        }
    }
    if (findFail == false)
    {
        resultX = p->x - myX;
        resultY = p->y - myY;
    }
    else {
        resultX = 0;
        resultY = 0;
    }

    for (auto p : Closed)
    {
       delete p;
    }
    for (int i = 0; i < 2000; ++i) 
    { 
        delete[] visit[i];
    } 
    
    delete[] visit;

}





