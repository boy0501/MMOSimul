Npc = require('script/NPCHelper')

myid = 99999;
automove = false;
move_cnt = 0;
is_active = true;
half_hp_trigger = false;
damage = 5;
myRange = 5;    --���� ��Ÿ�
move_limit = 3; --�̵� ��Ÿ�

spawnAreaCenterX = 20;   --�� ��ǥ�� �������� ���� �ٿ�������� �¾�� ���ٰ���.
spawnAreaCenterY = 20;  --���� ����.
spawnWidth = 600;   --���� ���� �ٿ����
spawnHeight = 200;  --���� ���� �ٿ����
spawnX = 20;   --spawn��ǥ �ǵ���������
spawnY = 20;  --spawn��ǥ �ǵ���������
Init_MaxHP = 20;    --�̰ɷ� �ʱ�ȭ�� ����
Init_Level = 1;     --�̰ɷ��ʱ�ȭ
Init_Name = "test";    --�̰ɷ� �ʱ�ȭ
Init_Accurate_Pos = true;  --false�� Area��ó��, true�� ��Ȯ�� ���ڸ���

NpcType = 0;    -- 0�̸� Peace, 1�̸� Agro
MonMoveType = 1;-- 0�̸� Siege, 1�̸� Move
interactfunc = nil;
user = {};
function Init()
    return NpcType,
    spawnAreaCenterX,
    spawnAreaCenterY,
    Init_Name,
    Init_Accurate_Pos;
end

function set_SpawnXY(x,y)
    spawnX = x;
    spawnY = y;
end

function get_SpawnXY()
    return spawnX,spawnY;
end

function set_uid(x)
   myid = x;
end

function set_is_active(active)
    is_active = active;
end

function get_Spawn_Pos()
    return spawnX,spawnY;
end

--���� �ڷ�ƾ ��� �Լ� 
interactfunc = function(player)
    local ret = Npc.API_NoticeWindowOK(myid,player,"Hi i am nimus Can you Hear my Story?");

    if(ret == 0) then
        API_NoticeWindow(myid,player, "You Push No Button");
    elseif(ret == 1) then
        API_NoticeWindow(myid,player, "You Push Yes button");
    else 
        API_NoticeWindow(myid,player, "err ");
    end
    API_NoticeWindow(myid,player, "Adding Text");
    API_NoticeWindow(myid,player, "Adding Text two");
    API_NoticeWindow(myid,player, "Adding Threee");

    Npc.API_NoticeWindowOK(myid,player,"Test Conversation Two");
    
end

--Ŭ���� �Ҹ��� �Լ� (�ʼ�)
function event_interaction(player)
    user[player] = Npc.API_DialogRequest(player,interactfunc);
end

--dialog����� �Ҹ��� �Լ� (�ʼ�)
function event_interaction_user_response(player, ret)
    Npc.API_DialogResponse(user[player],ret)    
end


function event_timer_ai(target)
    if (is_active == false) then
        return;
    end
   if(target == -1) then
       API_AutoMoveNPC(myid,spawnX,spawnY,move_limit);
       return;
   end
   
   API_SendMessageMySight(myid,"Hello",false);
end

function event_hit(player)
    if (is_active == false) then
        return;
    end

end