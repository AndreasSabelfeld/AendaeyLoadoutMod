modded class MissionServer
{
    override void OnInit()
    {
        super.OnInit();
        AendaeyLoadoutManager.Log("MissionServer.OnInit() -> Loading Loadouts on the Server...");
        AendaeyLoadoutManager.LoadLoadouts();
    }

    override void InvokeOnConnect(PlayerBase player, PlayerIdentity identity)
    {
        super.InvokeOnConnect(player, identity);

        if (player && identity)
        {
            // Fallback: Falls OnInit durch eine andere Mod übersprungen wurde, jetzt laden
            if (AendaeyLoadoutManager.m_AllLoadouts.Count() == 0)
            {
                AendaeyLoadoutManager.LoadLoadouts();
            }

            AendaeyLoadoutManager.Log("InvokeOnConnect: Sending " + AendaeyLoadoutManager.m_AllLoadouts.Count() + " Loadouts to " + identity.GetName());
            ScriptRPC rpc = new ScriptRPC();
            AendaeyLoadoutManager.WriteLoadoutsToRPC(rpc);
            rpc.Send(player, AendaeyLoadoutRPC.RPC_SYNC_LOADOUTS, true, identity);
        }
    }
}