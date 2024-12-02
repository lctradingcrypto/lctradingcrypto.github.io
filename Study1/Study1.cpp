#include "sierrachart.h"

SCDLLName("EMA Auto Trading Strategy")

SCSFExport scsf_EMAAutoTradingStrategy(SCStudyInterfaceRef sc)
{
    // Initialisation des param�tres
    if (sc.SetDefaults)
    {
        sc.GraphName = "EMA Auto Trading Strategy";
        sc.StudyDescription = "Automatically places market orders based on EMAs, with adjustable settings.";
        sc.AutoLoop = 0; // Mode manuel pour plus de contr�le
        sc.GraphRegion = 2;

        // Inputs pour les param�tres de la strat�gie
        sc.Input[0].Name = "Enable Auto Trading";
        sc.Input[0].SetYesNo(1); // Activ� par d�faut

        sc.Input[1].Name = "Number of Contracts";
        sc.Input[1].SetInt(1); // Nombre de contrats par d�faut

        sc.Input[2].Name = "Stop Loss (Ticks)";
        sc.Input[2].SetInt(20); // Stop loss par d�faut

        sc.Input[3].Name = "Target Profit (Ticks)";
        sc.Input[3].SetInt(40); // Target profit par d�faut

        sc.Input[4].Name = "EMA 1 Period";
        sc.Input[4].SetInt(9); // P�riode de l'EMA 1 par d�faut

        sc.Input[5].Name = "EMA 2 Period";
        sc.Input[5].SetInt(21); // P�riode de l'EMA 2 par d�faut

        sc.Input[6].Name = "Bars to Pause After Exit";
        sc.Input[6].SetInt(2); // Nombre de barres � attendre apr�s la sortie par d�faut

        return;
    }

    // R�cup�ration des param�tres utilisateur
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

        sc.DataStartIndex = max(sc.Input[4].GetInt(), sc.Input[5].GetInt()); // D�but des calculs
    }

    // Calcul des EMAs
    if (sc.Index >= EMA1Period) sc.ExponentialMovAvg(sc.BaseDataIn[SC_LAST], EMA1, EMA1Period);
    if (sc.Index >= EMA2Period) sc.ExponentialMovAvg(sc.BaseDataIn[SC_LAST], EMA2, EMA2Period);

    // V�rifie la position actuelle
    s_SCPositionData PositionData;
    sc.GetTradePosition(PositionData);

    int CurrentPositionQuantity = PositionData.PositionQuantity;

    // D�tecte si la position est ferm�e
    if (PreviousPositionQuantity > 0 && CurrentPositionQuantity == 0)
    {
        BarsSinceLastPosition = 0; // R�initialise le compteur de barres
        sc.AddMessageToLog((SCString("Position ferm�e. Attente de ") + SCString(BarsToPauseAfterExit) + " barres avant de reprendre."), 1);
    }

    // Met � jour la position pr�c�dente
    PreviousPositionQuantity = CurrentPositionQuantity;

    // Incr�mente le compteur de barres uniquement apr�s la premi�re barre compl�te
    if (CurrentPositionQuantity == 0 && BarsSinceLastPosition < BarsToPauseAfterExit && sc.GetBarHasClosedStatus(sc.Index) == BHCS_BAR_HAS_CLOSED)
    {
        BarsSinceLastPosition++;
    }

    if (EnableAutoTrading && CurrentPositionQuantity == 0 && BarsSinceLastPosition >= BarsToPauseAfterExit) // Auto-trading activ�, aucune position active, et suffisamment de barres �coul�es
    {
        float CurrentPrice = sc.BaseDataIn[SC_LAST][sc.Index];
        bool IsUpTrend = (EMA1[sc.Index] > EMA2[sc.Index]);
        bool IsDownTrend = (EMA1[sc.Index] < EMA2[sc.Index]);

        // Pr�pare l'ordre principal avec les ordres attach�s
        s_SCNewOrder Order;
        Order.OrderType = SCT_ORDERTYPE_MARKET;
        Order.TimeInForce = SCT_TIF_GTC; // Good Till Canceled
        Order.OrderQuantity = NumberOfContracts;
        Order.Target1Offset = TargetProfitTicks * sc.TickSize; // Target profit
        Order.Stop1Offset = StopLossTicks * sc.TickSize;       // Stop loss

        // Achat si la tendance est haussi�re
        if (IsUpTrend)
        {
            sc.BuyEntry(Order);
        }
        // Vente si la tendance est baissi�re
        else if (IsDownTrend)
        {
            sc.SellEntry(Order);
        }
    }
}

