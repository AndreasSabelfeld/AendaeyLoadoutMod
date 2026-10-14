modded class MissionGameplay
{
    ref LoadoutMenu m_LoadoutMenu;
    bool m_HasShownMenu = false;
    bool m_HasRequestedLoadouts = false;

    override void OnInit()
    {
        super.OnInit();

        if (!GetGame().IsMultiplayer())
        {
            AendaeyLoadoutManager.LoadLoadouts();
            AendaeyLoadoutManager.m_ClientHasSynced = true;
        }
    }

    override void OnUpdate(float timeslice)
    {
        super.OnUpdate(timeslice);

        PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());

        if (player && player.IsAlive())
        {
            // 1. Sobald der lokale Spieler existiert, Loadouts beim Server anfordern
            if (!m_HasRequestedLoadouts && !AendaeyLoadoutManager.m_ClientHasSynced)
            {
                m_HasRequestedLoadouts = true;
                AendaeyLoadoutManager.Log("Local player ready -> Requesting Loadouts from Server (RPC_REQUEST_LOADOUTS)...");

                ScriptRPC rpc = new ScriptRPC();
                rpc.Send(player, AendaeyLoadoutRPC.RPC_REQUEST_LOADOUTS, true, null);
            }

            // 2. Öffne das Menü, sobald mindestens 1 Loadout vom Server synchronisiert wurde
            if (AendaeyLoadoutManager.m_ClientHasSynced && AendaeyLoadoutManager.m_AllLoadouts.Count() > 0 && !m_HasShownMenu)
            {
                if (!GetGame().GetUIManager().GetMenu())
                {
                    AendaeyLoadoutManager.Log("Öffne LoadoutMenu auf dem Client...");
                    m_LoadoutMenu = new LoadoutMenu();
                    GetGame().GetUIManager().ShowScriptedMenu(m_LoadoutMenu, null);
                    m_HasShownMenu = true;
                }
            }
        }
        else if (!player || !player.IsAlive())
        {
            m_HasShownMenu = false;
            m_HasRequestedLoadouts = false;
        }
    }
}