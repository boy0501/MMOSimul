Npc = require('script/NPCHelper')

myid = 99999;
automove = false;
move_cnt = 0;
is_active = true;
half_hp_trigger = false;
damage = 5;
myRange = 5;    --공격 사거리
move_limit = 3; --이동 사거리

spawnAreaCenterX = 20;   --이 좌표를 기준으로 일정 바운더리에서 태어나게 해줄거임.
spawnAreaCenterY = 20;  --이하 동일.
spawnWidth = 600;   --스폰 일정 바운더리
spawnHeight = 200;  --스폰 일정 바운더리
spawnX = 20;   --spawn좌표 건들지마세요
spawnY = 20;  --spawn좌표 건들지마세요
Init_MaxHP = 20;    --이걸로 초기화할 생각
Init_Level = 1;     --이걸로초기화
Init_Name = "test";    --이걸로 초기화
Init_Accurate_Pos = true;  --false면 Area근처에, true면 정확히 그자리에

NpcType = 0;    -- 0이면 Peace, 1이면 Agro
MonMoveType = 1;-- 0이면 Siege, 1이면 Move
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

--그저 코루틴 덩어리 함수 
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

--클릭시 불리는 함수 (필수)
function event_interaction(player)
    user[player] = Npc.API_DialogRequest(player,interactfunc);
end

--dialog응답시 불리는 함수 (필수)
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
