
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

function event_interaction_0001_NPC(player)   
    --API_NoticeWindow(player,"Push Next Button");
    --ret = API_NoticeWindowOK(player,"Test Conversation");
    --if(ret == 1) then
    --    API_NoticeWindow(player, "You Push Yes Button");
    --else if(ret == 0) then
    --    API_NoticeWindow(player, "You Push No button");
    --end
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