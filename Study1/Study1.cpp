#include "sierrachart.h"

SCDLLName("EMA Auto Trading Strategy")

SCSFExport scsf_EMAAutoTradingStrategy(SCStudyInterfaceRef sc)
{
    // Initialisation des paramètres
    if (sc.SetDefaults)
    {
        sc.GraphName = "EMA Auto Trading Strategy";
        sc.StudyDescription = "Automatically places market orders based on EMAs, with adjustable settings.";
        sc.AutoLoop = 0; // Mode manuel pour plus de contrôle
        sc.GraphRegion = 2;

        // Inputs pour les paramètres de la stratégie
        sc.Input[0].Name = "Enable Auto Trading";
        sc.Input[0].SetYesNo(1); // Activé par défaut

        sc.Input[1].Name = "Number of Contracts";
        sc.Input[1].SetInt(1); // Nombre de contrats par défaut

        sc.Input[2].Name = "Stop Loss (Ticks)";
        sc.Input[2].SetInt(20); // Stop loss par défaut

        sc.Input[3].Name = "Target Profit (Ticks)";
        sc.Input[3].SetInt(40); // Target profit par défaut

        sc.Input[4].Name = "EMA 1 Period";
        sc.Input[4].SetInt(9); // Période de l'EMA 1 par défaut

        sc.Input[5].Name = "EMA 2 Period";
        sc.Input[5].SetInt(21); // Période de l'EMA 2 par défaut

        sc.Input[6].Name = "Bars to Pause After Exit";
        sc.Input[6].SetInt(2); // Nombre de barres à attendre après la sortie par défaut

        return;
    }

    // Récupération des paramètres utilisateur
    bool EnableAutoTrading = sc.Input[0].GetYesNo();
    int NumberOfContracts = sc.Input[1].GetInt();
    int StopLossTicks = sc.Input[2].GetInt();
    int TargetProfitTicks = sc.Input[3].GetInt();
    int EMA1Period = sc.Input[4].GetInt();
    int EMA2Period = sc.Input[5].GetInt();
    int BarsToPauseAfterExit = sc.Input[6].GetInt();

    // Variables internes pour surveiller les positions
    static int PreviousPositionQuantity = 0;
    static int BarsSinceLastPosition = 0;

    // Subgraphs pour les EMAs
    SCSubgraphRef EMA1 = sc.Subgraph[0];
    SCSubgraphRef EMA2 = sc.Subgraph[1];

    // Initialisation des Subgraphs
    if (sc.UpdateStartIndex == 0)
    {
        EMA1.Name = "EMA 1";
        EMA1.DrawStyle = DRAWSTYLE_LINE;
        EMA1.PrimaryColor = RGB(0, 255, 0);

        EMA2.Name = "EMA 2";
        EMA2.DrawStyle = DRAWSTYLE_LINE;
        EMA2.PrimaryColor = RGB(255, 0, 0);

        sc.DataStartIndex = max(sc.Input[4].GetInt(), sc.Input[5].GetInt()); // Début des calculs
    }

    // Calcul des EMAs
    if (sc.Index >= EMA1Period) sc.ExponentialMovAvg(sc.BaseDataIn[SC_LAST], EMA1, EMA1Period);
    if (sc.Index >= EMA2Period) sc.ExponentialMovAvg(sc.BaseDataIn[SC_LAST], EMA2, EMA2Period);

    // Vérifie la position actuelle
    s_SCPositionData PositionData;
    sc.GetTradePosition(PositionData);

    int CurrentPositionQuantity = PositionData.PositionQuantity;

    // Détecte si la position est fermée
    if (PreviousPositionQuantity > 0 && CurrentPositionQuantity == 0)
    {
        BarsSinceLastPosition = 0; // Réinitialise le compteur de barres
        sc.AddMessageToLog((SCString("Position fermée. Attente de ") + SCString(BarsToPauseAfterExit) + " barres avant de reprendre."), 1);
    }

    // Met à jour la position précédente
    PreviousPositionQuantity = CurrentPositionQuantity;

    // Incrémente le compteur de barres uniquement après la première barre complète
    if (CurrentPositionQuantity == 0 && BarsSinceLastPosition < BarsToPauseAfterExit && sc.GetBarHasClosedStatus(sc.Index) == BHCS_BAR_HAS_CLOSED)
    {
        BarsSinceLastPosition++;
    }

    if (EnableAutoTrading && CurrentPositionQuantity == 0 && BarsSinceLastPosition >= BarsToPauseAfterExit) // Auto-trading activé, aucune position active, et suffisamment de barres écoulées
    {
        float CurrentPrice = sc.BaseDataIn[SC_LAST][sc.Index];
        bool IsUpTrend = (EMA1[sc.Index] > EMA2[sc.Index]);
        bool IsDownTrend = (EMA1[sc.Index] < EMA2[sc.Index]);

        // Prépare l'ordre principal avec les ordres attachés
        s_SCNewOrder Order;
        Order.OrderType = SCT_ORDERTYPE_MARKET;
        Order.TimeInForce = SCT_TIF_GTC; // Good Till Canceled
        Order.OrderQuantity = NumberOfContracts;
        Order.Target1Offset = TargetProfitTicks * sc.TickSize; // Target profit
        Order.Stop1Offset = StopLossTicks * sc.TickSize;       // Stop loss

        // Achat si la tendance est haussière
        if (IsUpTrend)
        {
            sc.BuyEntry(Order);
        }
        // Vente si la tendance est baissière
        else if (IsDownTrend)
        {
            sc.SellEntry(Order);
        }
    }
}

