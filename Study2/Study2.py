import matplotlib.pyplot as plt
import matplotlib.animation as animation
import numpy as np

# Define data for the simulation
n_points = 100
x_data = np.arange(n_points)
entry_price = 100
tick_size = 0.25

# Generate simulated price data
last_trade_price = entry_price + np.cumsum(np.random.uniform(-1, 1, n_points) * tick_size)
close_price = entry_price + np.cumsum(np.random.uniform(-1, 1, n_points) * tick_size)

target_profit_ticks = 40
trigger_distance_ticks = 36
reverse_distance_ticks = 35

target_profit_price = entry_price + (target_profit_ticks * tick_size)
trigger_price = target_profit_price - (trigger_distance_ticks * tick_size)
reverse_price = trigger_price - (reverse_distance_ticks * tick_size)

target_profit_prices = np.full(n_points, target_profit_price)
trigger_prices = np.full(n_points, trigger_price)
reverse_prices = np.full(n_points, reverse_price)

# Create the animation
fig, ax = plt.subplots()
ax.set_xlim(0, n_points)
ax.set_ylim(entry_price - 20 * tick_size, entry_price + 50 * tick_size)

line_last_trade, = ax.plot([], [], label="Last Trade Price", color="cyan", linewidth=2)
line_close, = ax.plot([], [], label="Close Price", color="magenta", linewidth=2)
line_target_profit, = ax.plot([], [], label="Target Profit Price", color="green", linestyle="--")
line_trigger, = ax.plot([], [], label="Trigger Price", color="yellow", linestyle="--")
line_reverse, = ax.plot([], [], label="Reverse Price", color="red", linestyle="--")

ax.legend()

def init():
    line_last_trade.set_data([], [])
    line_close.set_data([], [])
    line_target_profit.set_data([], [])
    line_trigger.set_data([], [])
    line_reverse.set_data([], [])
    return line_last_trade, line_close, line_target_profit, line_trigger, line_reverse

def update(frame):
    line_last_trade.set_data(x_data[:frame], last_trade_price[:frame])
    line_close.set_data(x_data[:frame], close_price[:frame])
    line_target_profit.set_data(x_data[:frame], target_profit_prices[:frame])
    line_trigger.set_data(x_data[:frame], trigger_prices[:frame])
    line_reverse.set_data(x_data[:frame], reverse_prices[:frame])
    return line_last_trade, line_close, line_target_profit, line_trigger, line_reverse

ani = animation.FuncAnimation(fig, update, frames=n_points, init_func=init, blit=True)

# Save the video
video_path = "Tick_Based_Exit_Strategy_Simulation.mp4"
ani.save(video_path, fps=10, extra_args=['-vcodec', 'libx264'])

print(f"Video saved as {video_path}")

