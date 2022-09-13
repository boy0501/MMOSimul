
myid = 99999;
automove = false;
move_cnt = 0;
is_active = true;
half_hp_trigger = false;
damage = 5;     --���� ���ݷ�
ApplyDamage = 5;--���� ���ݷ� (�ǵ������ÿ�)
myRange = 1;    --���� ��Ÿ�
move_limit = 10; --�̵� ��Ÿ�
notice_range = 2;--��׷� ���� ��Ÿ�

spawnAreaCenterX = 976;   --�� ��ǥ�� �������� ���� �ٿ�������� �¾�� ���ٰ���.
spawnAreaCenterY = 1178;  --���� ����.
spawnWidth = 323;   --���� ���� �ٿ����
spawnHeight = 320;  --���� ���� �ٿ����
Init_Accurate_Pos = false;  --false�� Area��ó��, true�� ��Ȯ�� ���ڸ���
spawnX = 755;   --spawn��ǥ �ǵ���������
spawnY = 1454;  --spawn��ǥ �ǵ���������
Init_MaxHP = 20;    --�̰ɷ� �ʱ�ȭ�� ����
Init_Level = 1;     --�̰ɷ��ʱ�ȭ
Init_Name = "ANGRYMONSTER";    --�̰ɷ� �ʱ�ȭ
MonType = 1;    -- 0�̸� Peace, 1�̸� Agro
MonMoveType = 1;-- 0�̸� Siege, 1�̸� Move


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
