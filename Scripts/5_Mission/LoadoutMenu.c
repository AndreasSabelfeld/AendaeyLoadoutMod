class LoadoutMenu extends UIScriptedMenu
{
    protected Widget m_LoadoutContainer;
    protected ref map<Widget, string> m_ButtonLoadoutMap = new map<Widget, string>();

    override Widget Init()
    {
        AendaeyLoadoutManager.Log("LoadoutMenu.Init() is executing...");

        layoutRoot = GetGame().GetWorkspace().CreateWidgets("AendaeyLoadoutMod/GUI/layouts/LoadoutMenu.layout");
        if (!layoutRoot)
        {
            AendaeyLoadoutManager.Log("CRITICAL ERROR: LoadoutMenu.layout could not be loaded!");
            return null;
        }

        m_LoadoutContainer = layoutRoot.FindAnyWidget("LoadoutContainer");
        if (!m_LoadoutContainer)
        {
            AendaeyLoadoutManager.Log("CRITICAL ERROR: Widget 'LoadoutContainer' not found in LoadoutMenu.layout!");
            return layoutRoot;
        }

        PopulateLoadouts();

        return layoutRoot;
    }

    void PopulateLoadouts()
    {
        if (!m_LoadoutContainer)
            return;

        foreach (int idx, LoadoutConfig config : AendaeyLoadoutManager.m_AllLoadouts)
        {
            if (!config)
                continue;

            Widget itemWidget = GetGame().GetWorkspace().CreateWidgets("AendaeyLoadoutMod/GUI/layouts/LoadoutItem.layout", m_LoadoutContainer);
            if (!itemWidget)
                continue;

            itemWidget.SetFlags(WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE | WidgetFlags.VISIBLE);
            itemWidget.SetSize(300, 420);
            itemWidget.Show(true);

            // Falls noch ein altes PreviewWidget im Layout existiert, ausblenden/entfernen
            Widget oldPreview = itemWidget.FindAnyWidget("PreviewWidget");
            if (oldPreview)
            {
                oldPreview.Show(false);
                oldPreview.Unlink();
            }

            TextWidget nameText = TextWidget.Cast(itemWidget.FindAnyWidget("LoadoutNameText"));
            MultilineTextWidget descText = MultilineTextWidget.Cast(itemWidget.FindAnyWidget("DescriptionText"));
            ButtonWidget btn = ButtonWidget.Cast(itemWidget.FindAnyWidget("SelectButton"));

            if (nameText)
            {
                nameText.SetText(config.LoadoutName);
            }

            // Fallback: Falls das alte LoadoutItem.layout geladen wurde und DescriptionText noch nicht hat,
            // erzeugen wir es dynamisch im Code!
            if (!descText)
            {
                int flags = WidgetFlags.HEXACTPOS | WidgetFlags.VEXACTPOS | WidgetFlags.HEXACTSIZE | WidgetFlags.VEXACTSIZE | WidgetFlags.VISIBLE | WidgetFlags.IGNOREPOINTER;
                descText = MultilineTextWidget.Cast(GetGame().GetWorkspace().CreateWidget(MultilineTextWidgetTypeID, 20, 30, 260, 350, flags, ARGB(255, 220, 220, 220), 15, itemWidget));
            }

            if (descText)
            {
                descText.SetText(BuildLoadoutDescription(config));
                descText.Show(true);
            }

            if (btn)
            {
                m_ButtonLoadoutMap.Set(btn, config.LoadoutName);
            }
        }

        m_LoadoutContainer.Update();
    }

    protected string BuildLoadoutDescription(LoadoutConfig config)
    {
        string text = "";

        // 1. Optionale benutzerdefinierte Beschreibung aus der JSON
        if (config.Description != "")
        {
            text += config.Description + "\n\n";
        }

        // 2. Waffen auflisten
        if (config.Weapons && config.Weapons.Count() > 0)
        {
            text += "Weapons:\n";
            foreach (string wpn : config.Weapons)
            {
                if (wpn != "")
                {
                    text += " - " + GetCleanItemName(wpn) + "\n";
                }
            }
            text += "\n";
        }

        // 3. Kleidung / Ausrüstung auflisten
        if (config.Clothes && config.Clothes.Count() > 0)
        {
            text += "Equipment:\n";
            foreach (string cloth : config.Clothes)
            {
                if (cloth != "")
                {
                    text += " - " + GetCleanItemName(cloth) + "\n";
                }
            }
        }

        return text;
    }

    // Liest den lesbaren Ingame-Namen (displayName) aus der Config aus, falls vorhanden
    protected string GetCleanItemName(string className)
    {
        string displayName = "";

        if (GetGame().ConfigIsExisting("CfgWeapons " + className + " displayName"))
        {
            GetGame().ConfigGetText("CfgWeapons " + className + " displayName", displayName);
        }
        else if (GetGame().ConfigIsExisting("CfgVehicles " + className + " displayName"))
        {
            GetGame().ConfigGetText("CfgVehicles " + className + " displayName", displayName);
        }
        else if (GetGame().ConfigIsExisting("CfgMagazines " + className + " displayName"))
        {
            GetGame().ConfigGetText("CfgMagazines " + className + " displayName", displayName);
        }

        // Übersetze Stringtable-Keys (z.B. "$UNT$...") in echten Text
        if (displayName != "")
        {
            displayName = Widget.TranslateString(displayName);
            if (displayName != "" && !displayName.Contains("$UNT$"))
            {
                return displayName;
            }
        }

        return className;
    }

    override bool OnClick(Widget w, int x, int y, int button)
    {
        if (m_ButtonLoadoutMap.Contains(w))
        {
            string chosenLoadout = m_ButtonLoadoutMap.Get(w);
            AendaeyLoadoutManager.Log("Player has chosen loadout: '" + chosenLoadout + "' -> Sending RPC_APPLY_LOADOUT to Server...");

            PlayerBase player = PlayerBase.Cast(GetGame().GetPlayer());
            if (player)
            {
                ScriptRPC rpc = new ScriptRPC();
                rpc.Write(chosenLoadout);
                rpc.Send(player, AendaeyLoadoutRPC.RPC_APPLY_LOADOUT, true, null);
            }

            Close();
            return true;
        }

        return super.OnClick(w, x, y, button);
    }

    override void OnShow()
    {
        super.OnShow();
        GetGame().GetInput().ChangeGameFocus(1);
        GetGame().GetUIManager().ShowUICursor(true);
    }

    override void OnHide()
    {
        super.OnHide();
        GetGame().GetInput().ResetGameFocus();
        GetGame().GetUIManager().ShowUICursor(false);
    }
}