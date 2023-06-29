
myid = 99999;
automove = false;
move_cnt = 0;
is_active = true;
half_hp_trigger = false;
damage = 5;
myRange = 5;    --공격 사거리
move_limit = 3; --이동 사거리

spawnAreaCenterX = 650;   --이 좌표를 기준으로 일정 바운더리에서 태어나게 해줄거임.
spawnAreaCenterY = 1770;  --이하 동일.
spawnWidth = 600;   --스폰 일정 바운더리
spawnHeight = 200;  --스폰 일정 바운더리
spawnX = 755;   --spawn좌표 건들지마세요
spawnY = 1454;  --spawn좌표 건들지마세요
Init_MaxHP = 20;    --이걸로 초기화할 생각
Init_Level = 1;     --이걸로초기화
Init_Name = "Norm3";    --이걸로 초기화
Init_Accurate_Pos = false;  --false면 Area근처에, true면 정확히 그자리에

MonType = 0;    -- 0이면 Peace, 1이면 Agro
MonMoveType = 1;-- 0이면 Siege, 1이면 Move

function Init()
    return spawnHeight,
    spawnWidth,
    MonMoveType,
    MonType,
    Init_MaxHP,
    Init_Level,
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
    ret = API_NoticeWindowOK(player,"Test Conversation");
    if(ret == 1) then
        API_NoticeWindow(player, "You Push Yes Button");
    else if(ret == 0) then
        API_NoticeWindow(player, "You Push No button");
    end

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
