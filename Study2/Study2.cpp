
#include "sierrachart.h"

SCDLLName("Tick-Based Exit Strategy with Alerts")

SCSFExport scsf_TickBasedExitStrategyWithAlerts(SCStudyInterfaceRef sc)
{
    // Initialisation des paramètres par défaut
    if (sc.SetDefaults)
    {
        sc.GraphName = "Tick-Based Exit Strategy with Alerts";
        sc.StudyDescription = "Stratégie avec détection des conditions, alertes visuelles et journalisation.";
        sc.AutoLoop = 1; // Activer AutoLoop pour surveiller les ticks en continu

        // Inputs configurables
        sc.Input[0].Name = "Target Profit (Ticks)";
        sc.Input[0].SetInt(10); // Par défaut : 10 ticks

        sc.Input[1].Name = "Trigger Distance to Target (Ticks)";
        sc.Input[1].SetInt(2); // Par défaut : 2 ticks

        sc.Input[2].Name = "Reverse Distance (Ticks)";
        sc.Input[2].SetInt(11); // Par défaut : 11 ticks

        sc.Input[3].Name = "Enable Alerts";
        sc.Input[3].SetYesNo(1); // Par défaut : alertes activées

        return; // Fin de l'initialisation
    }

    // Récupérer les valeurs des inputs
    int TargetProfitTicks = sc.Input[0].GetInt();
    int TriggerDistanceToTarget = sc.Input[1].GetInt();
    int ReverseDistance = sc.Input[2].GetInt();
    bool EnableAlerts = sc.Input[3].GetYesNo();

    // Obtenir les données de la position actuelle
    s_SCPositionData PositionData;
    sc.GetTradePosition(PositionData);

    // Vérifier si une position est active
    if (PositionData.PositionQuantity == 0)
        return; // Aucune position ouverte, quitter la fonction

    float EntryPrice = PositionData.AveragePrice; // Prix moyen d'entrée
    float TargetProfitPrice;
    float TriggerPrice;
    float ReversePrice;

    // Calculs pour une position acheteuse
    if (PositionData.PositionQuantity > 0)
    {
        TargetProfitPrice = EntryPrice + (TargetProfitTicks * sc.TickSize);
        TriggerPrice = TargetProfitPrice - (TriggerDistanceToTarget * sc.TickSize);
        ReversePrice = TriggerPrice - (ReverseDistance * sc.TickSize);

        // Journalisation pour une position acheteuse
        if (EnableAlerts)
        {
            sc.AddMessageToLog(
                sc.FormatString("Achat actif : EntryPrice = %.2f, TargetProfitPrice = %.2f, ReversePrice = %.2f",
                                EntryPrice, TargetProfitPrice, ReversePrice),
                0
            );
        }

        // Vérifier les conditions pour sortir
        if (sc.Close[sc.Index] >= TriggerPrice && sc.Close[sc.Index] <= ReversePrice)
        {
            if (EnableAlerts)
            {
                sc.SetAlert(1, "Condition atteinte : Sortie de position acheteuse détectée.");
            }

            s_SCNewOrder ExitOrder;
            ExitOrder.OrderQuantity = PositionData.PositionQuantity; // Nombre total de contrats
            ExitOrder.OrderType = SCT_ORDERTYPE_MARKET; // Ordre au marché
            sc.SellOrder(ExitOrder);                   // Sortie de la position
        }
    }
    // Calculs pour une position vendeuse
    else if (PositionData.PositionQuantity < 0)
    {
        TargetProfitPrice = EntryPrice - (TargetProfitTicks * sc.TickSize);
        TriggerPrice = TargetProfitPrice + (TriggerDistanceToTarget * sc.TickSize);
        ReversePrice = TriggerPrice + (ReverseDistance * sc.TickSize);

        // Journalisation pour une position vendeuse
        if (EnableAlerts)
        {
            sc.AddMessageToLog(
                sc.FormatString("Vente active : EntryPrice = %.2f, TargetProfitPrice = %.2f, ReversePrice = %.2f",
                                EntryPrice, TargetProfitPrice, ReversePrice),
                0
            );
        }

        // Vérifier les conditions pour sortir
        if (sc.Close[sc.Index] <= TriggerPrice && sc.Close[sc.Index] >= ReversePrice)
        {
            if (EnableAlerts)
            {
                sc.SetAlert(2, "Condition atteinte : Sortie de position vendeuse détectée.");
            }

            s_SCNewOrder ExitOrder;
            ExitOrder.OrderQuantity = -PositionData.PositionQuantity; // Nombre total de contrats
            ExitOrder.OrderType = SCT_ORDERTYPE_MARKET;  // Ordre au marché
            sc.BuyOrder(ExitOrder);                     // Sortie de la position
        }
    }
}
