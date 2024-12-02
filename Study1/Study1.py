# -*- coding: utf-8 -*-
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np
import pandas as pd
import matplotlib as mpl
import matplotlib as mpl

# Définir le chemin de ffmpeg
mpl.rcParams['animation.ffmpeg_path'] = r"C:\Users\lctra\Desktop\ffmpeg-2024-11-28-git-bc991ca048-full_build\ffmpeg-2024-11-28-git-bc991ca048-full_build\bin\ffmpeg.exe"

# Param�tres de la strat�gie
n_points = 200
entry_price = 100
tick_size = 0.25
ema1_period = 9
ema2_period = 21
target_profit_ticks = 40
stop_loss_ticks = 20
bars_to_pause_after_exit = 2

# G�n�rer des prix simul�s
np.random.seed(42)
price_changes = np.random.uniform(-1, 1, n_points) * tick_size
price_data = entry_price + np.cumsum(price_changes)

# Calculer les EMA
price_series = pd.Series(price_data)
ema1 = price_series.ewm(span=ema1_period, adjust=False).mean()
ema2 = price_series.ewm(span=ema2_period, adjust=False).mean()

# Initialisation de la figure
fig, ax = plt.subplots()
ax.set_xlim(0, n_points)
ax.set_ylim(price_data.min() - 1, price_data.max() + 1)

line_price, = ax.plot([], [], label="Price", color="blue")
line_ema1, = ax.plot([], [], label=f"EMA {ema1_period}", color="green")
line_ema2, = ax.plot([], [], label=f"EMA {ema2_period}", color="red")

# Signaux d'achat et de vente
buy_signals = []
sell_signals = []
position = 0
bars_since_exit = bars_to_pause_after_exit

# Fonction pour l'animation
def init():
    line_price.set_data([], [])
    line_ema1.set_data([], [])
    line_ema2.set_data([], [])
    return line_price, line_ema1, line_ema2

def update(frame):
    global position, bars_since_exit

    # Mettre � jour les lignes de prix et EMA
    line_price.set_data(np.arange(frame), price_data[:frame])
    line_ema1.set_data(np.arange(frame), ema1[:frame])
    line_ema2.set_data(np.arange(frame), ema2[:frame])

    # D�terminer les signaux de trading
    if frame > max(ema1_period, ema2_period):
        if ema1[frame] > ema2[frame] and position == 0 and bars_since_exit >= bars_to_pause_after_exit:
            buy_signals.append((frame, price_data[frame]))
            position = 1  # Achat
            bars_since_exit = 0
        elif ema1[frame] < ema2[frame] and position == 0 and bars_since_exit >= bars_to_pause_after_exit:
            sell_signals.append((frame, price_data[frame]))
            position = -1  # Vente
            bars_since_exit = 0

        # Sortie de position (simplifi�e pour cette simulation)
        if position != 0 and bars_since_exit >= bars_to_pause_after_exit:
            position = 0
            bars_since_exit = 0

    # Incr�menter le compteur de barres depuis la derni�re sortie
    if position == 0 and bars_since_exit < bars_to_pause_after_exit:
        bars_since_exit += 1

    # Ajouter les signaux d'achat et de vente sur le graphique
    for signal in buy_signals:
        ax.plot(signal[0], signal[1], marker="^", color="green", markersize=10, label="Buy Signal")
    for signal in sell_signals:
        ax.plot(signal[0], signal[1], marker="v", color="red", markersize=10, label="Sell Signal")

    return line_price, line_ema1, line_ema2

ani = animation.FuncAnimation(fig, update, frames=n_points, init_func=init, blit=False)

# Sauvegarder la vid�o avec un writer alternatif si FFmpeg ne fonctionne pas
from matplotlib.animation import FFMpegWriter

try:
    Writer = animation.writers['ffmpeg']
    writer = Writer(fps=10, metadata=dict(artist='Me'), bitrate=1800)
    ani.save("EMA_Auto_Trading_Simulation.mp4", writer=writer)
    print("Vid�o enregistr�e avec succ�s sous le nom EMA_Auto_Trading_Simulation.mp4")
except FileNotFoundError:
    print("FFmpeg non trouv�. Veuillez v�rifier que FFmpeg est install� et accessible depuis le PATH.")
except Exception as e:
    print(f"Une erreur s'est produite : {e}")