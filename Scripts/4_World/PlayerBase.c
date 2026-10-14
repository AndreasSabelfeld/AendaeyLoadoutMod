modded class PlayerBase
{
    void SyncWeaponFSM(EntityAI weaponEnt)
    {
        Weapon_Base wpn;
        if (Class.CastTo(wpn, weaponEnt))
        {
            wpn.RandomizeFSMState();
            wpn.Synchronize();
        }
    }

    override void OnRPC(PlayerIdentity sender, int rpc_type, ParamsReadContext ctx)
    {
        super.OnRPC(sender, rpc_type, ctx);

        // 1. CLIENT empfängt die Loadout-Liste vom Server
        if (GetGame().IsClient() && rpc_type == AendaeyLoadoutRPC.RPC_SYNC_LOADOUTS)
        {
            AendaeyLoadoutManager.Log("Client receives RPC_SYNC_LOADOUTS from Server...");
            AendaeyLoadoutManager.ReadLoadoutsFromRPC(ctx);
        }

        // 2. SERVER empfängt Anfrage vom Client, die Loadouts zu senden
        if (GetGame().IsServer() && rpc_type == AendaeyLoadoutRPC.RPC_REQUEST_LOADOUTS)
        {
            if (AendaeyLoadoutManager.m_AllLoadouts.Count() == 0)
            {
                AendaeyLoadoutManager.LoadLoadouts();
            }

            ScriptRPC rpc = new ScriptRPC();
            AendaeyLoadoutManager.WriteLoadoutsToRPC(rpc);
            rpc.Send(this, AendaeyLoadoutRPC.RPC_SYNC_LOADOUTS, true, sender);
        }

        // 3. SERVER wendet das gewählte Loadout auf den Spieler an
        if (GetGame().IsServer() && rpc_type == AendaeyLoadoutRPC.RPC_APPLY_LOADOUT)
        {
            string chosenLoadout;
            if (!ctx.Read(chosenLoadout))
                return;

            ApplyChosenLoadout(chosenLoadout);
        }
    }

    void ApplyChosenLoadout(string loadoutName)
    {
        LoadoutConfig chosenConfig = null;
        foreach (LoadoutConfig cfg : AendaeyLoadoutManager.m_AllLoadouts)
        {
            if (cfg && cfg.LoadoutName == loadoutName)
            {
                chosenConfig = cfg;
                break;
            }
        }

        if (!chosenConfig)
        {
            AendaeyLoadoutManager.Log("ERROR: Loadout '" + loadoutName + "' not found on server!");
            return;
        }

        AendaeyLoadoutManager.Log("Applying loadout '" + loadoutName + "' to player...");

        // 1. Inventar komplett leeren
        RemoveAllItems();

        // 2. Kleidung anziehen + Spezial-Attachments für Mich2001Helmet & PlateCarrierVest
        foreach (string clothName : chosenConfig.Clothes)
        {
            if (clothName == "")
                continue;

            EntityAI clothObj = GetInventory().CreateAttachment(clothName);
            if (!clothObj)
            {
                clothObj = GetInventory().CreateInInventory(clothName);
            }

            if (clothObj)
            {
                if (clothName == "Mich2001Helmet")
                {
                    EntityAI nvg = clothObj.GetInventory().CreateAttachment("NVGoggles");
                    if (nvg)
                    {
                        nvg.GetInventory().CreateAttachment("Battery9V");
                    }
                }
                else if (clothName == "PlateCarrierVest")
                {
                    clothObj.GetInventory().CreateAttachment("PlateCarrierPouches");
                    clothObj.GetInventory().CreateAttachment("PlateCarrierHolster");
                }
            }
        }

        // 3. Medizin & Granaten
        EntityAI bandage = GetInventory().CreateInInventory("BandageDressing");
        GetInventory().CreateInInventory("BandageDressing");
        EntityAI epi = GetInventory().CreateInInventory("Epinephrine");
        EntityAI morph = GetInventory().CreateInInventory("Morphine");
        EntityAI grenade = GetInventory().CreateInInventory("M67Grenade");
        GetInventory().CreateInInventory("M67Grenade");

        // 4. Primärwaffe + Attachments + Magazine + 1.29 FSM Fix
        EntityAI primaryWeapon = null;
        if (chosenConfig.Weapons && chosenConfig.Weapons.Count() > 0 && chosenConfig.Weapons[0] != "")
        {
            string primName = chosenConfig.Weapons[0];

            // Automatischer Fallback für FamasF1 / AugOptic
            if (primName == "FamasF1" && !GetGame().ConfigIsExisting("CfgWeapons FamasF1"))
                primName = "Famas";
            if (primName == "AugOptic")
                primName = "AugShort";

            primaryWeapon = GetInventory().CreateAttachment(primName);
            if (!primaryWeapon)
            {
                primaryWeapon = GetInventory().CreateInInventory(primName);
            }

            if (primaryWeapon)
            {
                // Waffen-Attachments (Schäfte, Handschutz, Scopes)
                if (chosenConfig.PrimaryAttachments)
                {
                    foreach (string attName : chosenConfig.PrimaryAttachments)
                    {
                        if (attName != "")
                        {
                            EntityAI wpnAtt = primaryWeapon.GetInventory().CreateAttachment(attName);
                            if (wpnAtt)
                            {
                                // Falls das Visier eine Batterie braucht (ACOG braucht keine, M68/Kobra/PSO brauchen 9V)
                                wpnAtt.GetInventory().CreateAttachment("Battery9V");
                            }
                        }
                    }
                }

                // Magazin an die Waffe + 4 Ersatzmagazine ins Inventar
                if (chosenConfig.PrimaryMag != "")
                {
                    primaryWeapon.GetInventory().CreateAttachment(chosenConfig.PrimaryMag);
                    for (int m = 0; m < 4; m++)
                    {
                        GetInventory().CreateInInventory(chosenConfig.PrimaryMag);
                    }
                }
                else if (chosenConfig.PrimaryAmmo != "")
                {
                    for (int a = 0; a < 4; a++)
                    {
                        GetInventory().CreateInInventory(chosenConfig.PrimaryAmmo);
                    }
                }

                SyncWeaponFSM(primaryWeapon);
            }
        }

        // 5. Sekundärwaffe (Pistole) + Magazin + 2 Ersatzmagazine + 1.29 FSM Fix
        EntityAI secondaryWeapon = null;
        if (chosenConfig.Weapons && chosenConfig.Weapons.Count() > 1 && chosenConfig.Weapons[1] != "")
        {
            string secName = chosenConfig.Weapons[1];
            secondaryWeapon = GetInventory().CreateInInventory(secName);

            string secMag = chosenConfig.SecondaryMag;
            if (secMag == "")
            {
                if (secName == "FNX45") secMag = "Mag_FNX45_15Rnd";
                else if (secName == "Glock19") secMag = "Mag_Glock_15Rnd";
            }

            if (secondaryWeapon && secMag != "")
            {
                secondaryWeapon.GetInventory().CreateAttachment(secMag);
                SyncWeaponFSM(secondaryWeapon);
            }

            if (secMag != "")
            {
                GetInventory().CreateInInventory(secMag);
                GetInventory().CreateInInventory(secMag);
            }
        }

        // 6. Quickbar zuweisen
        if (primaryWeapon) SetQuickBarEntityShortcut(primaryWeapon, 0);
        if (secondaryWeapon) SetQuickBarEntityShortcut(secondaryWeapon, 1);
        if (bandage) SetQuickBarEntityShortcut(bandage, 2);
        if (epi) SetQuickBarEntityShortcut(epi, 3);
        if (morph) SetQuickBarEntityShortcut(morph, 4);
        if (grenade) SetQuickBarEntityShortcut(grenade, 5);
    }
}