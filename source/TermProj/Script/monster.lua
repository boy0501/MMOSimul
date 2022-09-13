
myid = 99999;
automove = false;
move_cnt = 0;
is_active = true;
half_hp_trigger = false;
damage = 5;
myRange = 3;
move_limit = 5; --이동 사거리 - Peace몬스터는 이 값이 감지 반경으로 사용된다.

spawnAreaX = 1000;   --이 좌표를 기준으로 일정 바운더리에서 태어나게 해줄거임.
spawnAreaY = 1000;  --이하 동일.
spawnX = 755;   --spawn좌표 건들지마세요
spawnY = 1454;  --spawn좌표 건들지마세요
Init_MaxHP = 20;    --이걸로 초기화할 생각
Init_Level = 1;     --이걸로초기화
Init_Name = "Plant";    --이걸로 초기화
Init_Accurate_Pos = false;  --false면 Area근처에, true면 정확히 그자리에

MonType = 0;    -- 0이면 Peace, 1이면 Agro
MonMoveType = 0;-- 0이면 Siege, 1이면 Move

function Init()
    return move_limit,MonMoveType,MonType,Init_MaxHP,Init_Level,spawnAreaX,spawnAreaY,Init_Name,Init_Accurate_Pos;
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

function event_timer_ai(target)
    if (is_active == false) then
        return;
    end

    if(target == -1) then
        return;
    end

    API_MonsterAttack(myid,target,damage,myRange);
end

function event_hit(player)
    if (is_active == false) then
        return;
    end
    my_hp = API_get_hp(myid);
    my_MaxHp = API_get_MaxHp(myid);
    if(half_hp_trigger == false) then
        if(my_hp <= my_MaxHp * 0.5) then
            half_hp_trigger = true;
            API_SendMessageMySight(myid,"Why you Bother Me!");
        end
    end
    if (my_hp <= 0) then
        move_cnt = 0;
        automove = false;
        half_hp_trigger = false;
        is_active = false;
        API_MonsterDie(myid,player);
    end
end
