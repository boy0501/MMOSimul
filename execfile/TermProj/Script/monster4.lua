
myid = 99999;
automove = false;
move_cnt = 0;
is_active = true;
half_hp_trigger = false;
damage = 5;     --고정 공격력
ApplyDamage = 5;--최종 공격력 (건들지마시오)
myRange = 1;    --공격 사거리
move_limit = 10; --이동 사거리
notice_range = 2;--어그로 감지 사거리

spawnAreaCenterX = 976;   --이 좌표를 기준으로 일정 바운더리에서 태어나게 해줄거임.
spawnAreaCenterY = 1178;  --이하 동일.
spawnWidth = 323;   --스폰 일정 바운더리
spawnHeight = 320;  --스폰 일정 바운더리
Init_Accurate_Pos = false;  --false면 Area근처에, true면 정확히 그자리에
spawnX = 755;   --spawn좌표 건들지마세요
spawnY = 1454;  --spawn좌표 건들지마세요
Init_MaxHP = 20;    --이걸로 초기화할 생각
Init_Level = 1;     --이걸로초기화
Init_Name = "ANGRYMONSTER";    --이걸로 초기화
MonType = 1;    -- 0이면 Peace, 1이면 Agro
MonMoveType = 1;-- 0이면 Siege, 1이면 Move


function Init()
    return spawnHeight,spawnWidth,MonMoveType,MonType,notice_range,Init_MaxHP,Init_Level,spawnAreaCenterX,spawnAreaCenterY,Init_Name,Init_Accurate_Pos;
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


function event_timer_ai(target)
    if (is_active == false) then
        return;
    end

    if(target == -1) then
        API_AutoMoveNPC(myid,spawnX,spawnY,move_limit);
        return;
    end

    target_x = API_get_x(target);
    target_y = API_get_y(target);
    my_x = API_get_x(myid);
    my_y = API_get_y(myid);

    if(target_x == my_x) then
        if(target_y == my_y) then
            API_MonsterAttack(myid,target,ApplyDamage,1)
            return;
        end
    end

    API_ChaseTarget(myid,target,spawnX,spawnY,move_limit);
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
            API_SendMessageMySight(myid,"Ughhhh!!!",true);
            ApplyDamage = math.floor(damage * 1.5);
        end
    end


    if (my_hp <= 0) then
        half_hp_trigger = false;
        is_active = false;
        ApplyDamage = damage;
        API_MonsterDie(myid,player);
    end
end
