
myid = 99999;
automove = false;
move_cnt = 0;
is_active = true;
half_hp_trigger = false;
damage = 5;
myRange = 3;
move_limit = 5; --�̵� ��Ÿ� - Peace���ʹ� �� ���� ���� �ݰ����� ���ȴ�.

spawnAreaX = 1000;   --�� ��ǥ�� �������� ���� �ٿ�������� �¾�� ���ٰ���.
spawnAreaY = 1000;  --���� ����.
spawnX = 755;   --spawn��ǥ �ǵ���������
spawnY = 1454;  --spawn��ǥ �ǵ���������
Init_MaxHP = 20;    --�̰ɷ� �ʱ�ȭ�� ����
Init_Level = 1;     --�̰ɷ��ʱ�ȭ
Init_Name = "Plant";    --�̰ɷ� �ʱ�ȭ
Init_Accurate_Pos = false;  --false�� Area��ó��, true�� ��Ȯ�� ���ڸ���

MonType = 0;    -- 0�̸� Peace, 1�̸� Agro
MonMoveType = 0;-- 0�̸� Siege, 1�̸� Move

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
