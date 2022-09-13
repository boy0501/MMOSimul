
myid = 99999;
automove = false;
move_cnt = 0;
is_active = true;
half_hp_trigger = false;
damage = 5;
myRange = 3;
function set_uid(x)
   myid = x;
end

function set_is_active(active)
    is_active = active;
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

function event_timer_ai(player)
    if (is_active == false) then
        return;
    end
    local p = player;
    if(move_cnt == 3) then
        API_SendMessage(myid,p,"BYE");
        move_cnt = 0;
        automove = false;
    end

    if(automove == true) then
        API_AutoMoveNPC(myid);
        move_cnt = move_cnt + 1;
    end
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
            API_SendMessageMySight(myid,"Why you Bother Me!",false);
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

function event_attack(target)
    if (is_active == false) then
        return;
    end

    API_MonsterAttack(myid,target,damage,myRange);
end