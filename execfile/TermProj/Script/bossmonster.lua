
myid = 99999;
automove = false;
move_cnt = 0;
is_active = true;
half_hp_trigger = false;
first_meet = false;
damage = 10;     --���� ���ݷ�
ApplyDamage = damage;--���� ���ݷ� (�ǵ������ÿ�)
myRange = 2;    --���� ��Ÿ�
move_limit = 6; --�̵� ��Ÿ� - Peace���ʹ� �� ���� ���� �ݰ����� ���ȴ�.
attack_Pattern = 0;
MonType = 0;    -- 0�̸� Peace, 1�̸� Agro
MonMoveType = 0;-- 0�̸� Siege, 1�̸� Move

spawnAreaX = 741;   --�� ��ǥ�� �������� ���� �ٿ�������� �¾�� ���ٰ���.
spawnAreaY = 1237;  --���� ����.
spawnX = 755;   --spawn��ǥ �ǵ���������
spawnY = 1454;  --spawn��ǥ �ǵ���������
Init_MaxHP = 50;    --�̰ɷ� �ʱ�ȭ�� ����
Init_Level = 5;     --�̰ɷ��ʱ�ȭ
Init_Name = "GHOSTBOSS";    --�̰ɷ� �ʱ�ȭ
Init_Accurate_Pos = true;  --false�� Area��ó��, true�� ��Ȯ�� ���ڸ���

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

function get_Spawn_Pos()
    return spawnX,spawnY;
end

function event_player_move(player)
   
   if (is_active == false) then
        return;
   end
   player_x = API_get_x(player);
   player_y = API_get_y(player);
   my_x = API_get_x(myid);
   my_y = API_get_y(myid);

   if (automove == false) then
      if (player_x == my_x) then
         if (player_y == my_y) then
            API_SendMessage(myid, player, "HELLO");
            API_AutoMoveNPC(myid, player);
            move_cnt = move_cnt + 1;
            automove = true;
         end
      end
   end
end

function event_timer_ai(target)
    if (is_active == false) then
        return;
    end

    if(target == -1) then
        return;
    end

    attack_Pattern =math.random(0,2);

    if(attack_Pattern == 0) then
        API_SendMessageMySight(myid,"aTtack~",false);
        API_MonsterAttack(myid,target,ApplyDamage,myRange);
    elseif ( attack_Pattern == 1) then
        API_TelePortTarget(myid, target);
        API_SendMessageMySight(myid,"hi~",false);
        API_MonsterAttack(myid,target,ApplyDamage,myRange);
    elseif ( attack_Pattern == 2) then
        Pattern_3Count = 0;
        API_SendMessageMySight(myid,"Kiyaaaaakk!!",false);
        API_BossDeBuff(myid,target,1,3);
    end
end

function event_hit(player)
    if (is_active == false) then
        return;
    end

    if ( first_meet == false) then
        API_SendMessageMySight(myid,"WHO WAKES ME UP!!!",false);
        first_meet = true;
    end

    my_hp = API_get_hp(myid);
    my_MaxHp = API_get_MaxHp(myid);
    if(half_hp_trigger == false) then
        if(my_hp <= my_MaxHp * 0.5) then
            half_hp_trigger = true;
            API_SendMessageMySight(myid,"Sssghhhhhhh",true);
            ApplyDamage = math.floor(damage * 1.5);
        end
    end


    if (my_hp <= 0) then
        half_hp_trigger = false;
        is_active = false;
        first_meet = false
        ApplyDamage = damage;
        API_SendMessageMySight(myid,"I will be Back...",false);
        API_BossBuffMySight(myid,1,10);
        API_MonsterDie(myid,player);
    end
end
