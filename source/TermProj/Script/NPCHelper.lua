local Npc = {}

function Npc.API_NoticeWindowOK(myid,player,text)

    API_NoticeWindowOK(myid,player,text);
    ret = coroutine.yield()
    return ret
end

function Npc.API_NoticeWindow(ret,player,text)

    API_NoticeWindow(ret,player,text)
end

function Npc.API_DialogResponse(cohandle,ret)
   coroutine.resume(cohandle,ret);    
end

function Npc.API_DialogRequest(player,foo)
    local cohandle = coroutine.create(foo);
    coroutine.resume(cohandle,player);
    return cohandle    
end


return Npc