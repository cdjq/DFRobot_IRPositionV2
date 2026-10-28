# -*- coding: utf-8 -*
'''!
@file  get_position.py
@brief This example reads up to 16 IR objects from the SEN0701 module and shows coordinates and metrics.
@n     Demonstrates begin(), optional parameter tuning (with valid ranges), update() per frame,
@n     and per-object accessors. See README "Configuration parameter ranges" for limits.
@n
@n     Two display modes, selected by DISPLAY_MODE below:
@n       DISPLAY_CONSOLE  -> print all 16 slots to the terminal every frame (works on headless / Lite systems)
@n       DISPLAY_GUI      -> tkinter window that draws the spots on a 2D canvas, with sliders for
@n                           brightness threshold / noise threshold / exposure (needs a desktop or ssh -X).
@n                           The same 16 slots are also printed to the terminal.
@n     If tkinter or a display is not available the example falls back to the console mode automatically.
@copyright   Copyright (c) 2026 DFRobot Co.Ltd (http://www.dfrobot.com)
@license     The MIT License (MIT)
@author      [thdyyl](yuanlong.yu@dfrobot.com)
@version     V1.0.0
@date        2026-10-26
@url         https://github.com/DFRobot/DFRobot_IRPositionV2
'''

from __future__ import print_function
import sys
import os

sys.path.append("../")
import time

from DFRobot_IRPositionV2 import *

'''
  I2C bus and 7-bit address of the bridge
  I2C_1   -> /dev/i2c-1 (default on Raspberry Pi)
  ADDRESS -> 0x58 (factory default, see set_i2c_address.py to change it)
'''
I2C_1 = 0x01
ADDRESS = 0x58
ir_position = DFRobot_IRPositionV2(I2C_1, ADDRESS)

'''
  Select display mode
  DISPLAY_CONSOLE -> text output in the terminal
  DISPLAY_GUI     -> tkinter canvas (falls back to console if unavailable)
'''
DISPLAY_CONSOLE = 0
DISPLAY_GUI = 1
DISPLAY_MODE = DISPLAY_GUI

'''
  Demo parameters. Valid ranges, see README "Configuration parameter ranges":
    DEMO_BRIGHTNESS_THRES: 0-255 (typ. default 0x97)
    DEMO_NOISE_THRES:      0-255 (typ. default 0x0A)
    DEMO_SCALE_RESOLUTION: each axis 0-4095 (maps native 98x98; 2490 ~ 30x scale)
    DEMO_MAX_TRACKING:     1-16
    DEMO_EXPOSURE:         100-65535, adjust for your lighting
'''
DEMO_BRIGHTNESS_THRES = 0xEE
DEMO_NOISE_THRES = 0xFF
DEMO_SCALE_RESOLUTION = 2490
DEMO_MAX_TRACKING = 16
DEMO_EXPOSURE = 0x0080

## Native sensor grid used by get_ir_bound()
NATIVE_GRID = 98


def setup():
  while ir_position.begin() == False:
    print("IRPositionV2 init failed: check wiring, pull-ups, and bridge firmware.")
    time.sleep(1)
  print("IRPositionV2 init OK.")

  ir_position.set_indicator(False)
  ir_position.set_ir_brightness_threshold(DEMO_BRIGHTNESS_THRES)  # 0-255
  ir_position.set_ir_detection_noise(DEMO_NOISE_THRES)  # 0-255
  ir_position.set_scale_resolution(DEMO_SCALE_RESOLUTION, DEMO_SCALE_RESOLUTION)  # x,y each 0-4095
  ir_position.set_max_tracking_number(DEMO_MAX_TRACKING)  # 1-16

  # set_exposure(expo): expo 100-65535. Waits a fixed 100 ms after a successful write.
  if ir_position.set_exposure(DEMO_EXPOSURE):
    print("set_exposure OK, wrote 0x%04X" % DEMO_EXPOSURE)
  else:
    print("set_exposure failed (I2C).")
  expo = ir_position.get_exposure()
  if expo >= 0:
    print("Exposure(readback)=0x%04X" % expo)
  else:
    print("get_exposure() failed.")


def is_valid_spot(x, y, avg_b):
  '''!
  @brief Decide whether an object slot holds a tracked spot
  @details Empty slots report zero brightness; coordinates outside the scaled range are also rejected.
  '''
  return avg_b > 0 and x <= DEMO_SCALE_RESOLUTION and y <= DEMO_SCALE_RESOLUTION


# ---------------------------------------------------------------- console mode
LOG_INTERVAL_S = 0.1


def print_frame():
  '''!
  @brief Print all 16 slots from the last update() to the terminal
  '''
  for i in range(16):
    # IR index i: 0-15
    x, y = ir_position.get_ir_coordinate(i)
    up, down, left, right = ir_position.get_ir_bound(i)
    avg_b = ir_position.get_average_brightness(i)
    max_b = ir_position.get_max_brightness(i)
    rad = ir_position.get_radius(i)
    print("Obj[%2d] Cxy=(%4d,%4d) AvgB=%3d MaxB=%3d R=%2d Bound(U,D,L,R)=(%d,%d,%d,%d)" % (i, x, y, avg_b, max_b, rad, up, down, left, right))
  print("----------------------------------------------------------------")


def loop():
  ## Read one frame, then access all 16 slots from the cached buffer
  if ir_position.update() == False:
    print("update() failed - frame read error.")
    time.sleep(0.5)
    return

  print_frame()
  time.sleep(LOG_INTERVAL_S)


# ---------------------------------------------------------------- GUI mode
def _load_tk():
  '''!
  @brief Import tkinter, accepting the Python 2 module name as well
  @return The tkinter module, or None when it is not installed
  '''
  try:
    import tkinter

    return tkinter
  except ImportError:
    pass
  try:
    import Tkinter

    return Tkinter
  except ImportError:
    return None


tk = _load_tk()


class PositionGUI:
  '''!
  @brief tkinter viewer: draws every tracked spot on a square canvas and exposes tuning sliders
  @details Canvas X/Y map the scaled 0..DEMO_SCALE_RESOLUTION range; boundary boxes map the native 0..98 grid.
  '''

  CANVAS_SIZE = 600
  REFRESH_MS = 20
  SPOT_COLORS = ["#ff5252", "#ffab40", "#ffd740", "#69f0ae", "#40c4ff", "#7c4dff", "#ff4081", "#ff6e40", "#eeff41", "#64ffda", "#448aff", "#b388ff", "#f06292", "#ffb74d", "#aed581", "#4fc3f7"]

  def __init__(self, root):
    self.root = root
    self.root.title("DFRobot IRPosition V2 - Get Position")
    self.root.resizable(False, False)

    self.frames = 0
    self.fps = 0.0
    self.fps_t0 = time.time()
    self.last_log = 0

    ## Left: canvas
    self.canvas = tk.Canvas(root, width=self.CANVAS_SIZE, height=self.CANVAS_SIZE, bg="#101418", highlightthickness=0)
    self.canvas.grid(row=0, column=0, rowspan=20, padx=8, pady=8)
    self._draw_grid()

    ## Right: controls
    panel = tk.Frame(root)
    panel.grid(row=0, column=1, sticky="n", padx=8, pady=8)

    self.var_bright = tk.IntVar(value=DEMO_BRIGHTNESS_THRES)
    self.var_noise = tk.IntVar(value=DEMO_NOISE_THRES)
    self.var_expo = tk.IntVar(value=DEMO_EXPOSURE)
    self.var_bound = tk.IntVar(value=1)
    self.var_led = tk.IntVar(value=0)

    self._add_slider(panel, "Brightness threshold (0-255)", self.var_bright, 0, 255, self._apply_bright)
    self._add_slider(panel, "Noise threshold (0-255)", self.var_noise, 0, 255, self._apply_noise)
    self._add_slider(panel, "Exposure (100-65535)", self.var_expo, 100, 0xFFFF, self._apply_expo)

    tk.Checkbutton(panel, text="Show boundary box", variable=self.var_bound).pack(anchor="w", pady=(8, 0))
    tk.Checkbutton(panel, text="Indicator LED", variable=self.var_led, command=self._apply_led).pack(anchor="w")

    tk.Label(panel, text="").pack()
    self.lbl_count = tk.Label(panel, text="Tracked spots: 0", anchor="w")
    self.lbl_count.pack(fill="x")
    self.lbl_fps = tk.Label(panel, text="FPS: --", anchor="w")
    self.lbl_fps.pack(fill="x")
    self.lbl_status = tk.Label(panel, text="Status: OK", anchor="w", fg="#2e7d32")
    self.lbl_status.pack(fill="x")

    ## Per-spot table
    tk.Label(panel, text="").pack()
    self.spot_text = tk.Text(panel, width=42, height=18, font=("Courier", 9), state="disabled")
    self.spot_text.pack()

    self.root.after(self.REFRESH_MS, self.refresh)

  def _add_slider(self, parent, label, var, lo, hi, on_release):
    tk.Label(parent, text=label, anchor="w").pack(fill="x", pady=(8, 0))
    s = tk.Scale(parent, from_=lo, to=hi, orient="horizontal", variable=var, length=260)
    s.pack(fill="x")
    # Send to the module only when the knob is released, to avoid flooding the I2C bus.
    s.bind("<ButtonRelease-1>", lambda e: on_release())

  def _draw_grid(self):
    step = self.CANVAS_SIZE / 10.0
    for i in range(1, 10):
      p = int(i * step)
      self.canvas.create_line(p, 0, p, self.CANVAS_SIZE, fill="#1f262e")
      self.canvas.create_line(0, p, self.CANVAS_SIZE, p, fill="#1f262e")

  def _set_status(self, ok, text):
    self.lbl_status.config(text="Status: " + text, fg="#2e7d32" if ok else "#c62828")

  def _apply_bright(self):
    ok = ir_position.set_ir_brightness_threshold(self.var_bright.get())
    self._set_status(ok, "brightness threshold -> %d" % self.var_bright.get() if ok else "I2C write failed")

  def _apply_noise(self):
    ok = ir_position.set_ir_detection_noise(self.var_noise.get())
    self._set_status(ok, "noise threshold -> %d" % self.var_noise.get() if ok else "I2C write failed")

  def _apply_expo(self):
    ok = ir_position.set_exposure(self.var_expo.get())
    self._set_status(ok, "exposure -> 0x%04X" % self.var_expo.get() if ok else "I2C write failed")

  def _apply_led(self):
    ok = ir_position.set_indicator(self.var_led.get() == 1)
    self._set_status(ok, "indicator %s" % ("on" if self.var_led.get() else "off") if ok else "I2C write failed")

  def _to_canvas(self, v):
    '''!
    @brief Map a scaled coordinate 0..DEMO_SCALE_RESOLUTION to canvas pixels
    '''
    return v * self.CANVAS_SIZE / float(DEMO_SCALE_RESOLUTION)

  def _bound_to_canvas(self, v):
    '''!
    @brief Map a native grid cell 0..98 to canvas pixels
    '''
    return v * self.CANVAS_SIZE / float(NATIVE_GRID)

  def refresh(self):
    if ir_position.update() == False:
      print("update() failed - frame read error.")
      self._set_status(False, "update() failed - frame read error")
      self.root.after(500, self.refresh)
      return

    self.canvas.delete("spot")
    lines = []
    tracked = 0
    for i in range(16):
      x, y = ir_position.get_ir_coordinate(i)
      up, down, left, right = ir_position.get_ir_bound(i)
      avg_b = ir_position.get_average_brightness(i)
      max_b = ir_position.get_max_brightness(i)
      rad = ir_position.get_radius(i)

      if not is_valid_spot(x, y, avg_b):
        continue
      tracked += 1
      color = self.SPOT_COLORS[i % len(self.SPOT_COLORS)]
      lines.append("%2d (%4d,%4d) A=%3d M=%3d R=%2d" % (i, x, y, avg_b, max_b, rad))

      cx = self._to_canvas(x)
      cy = self._to_canvas(y)
      # Radius field is 0-15 native pixels; scale it to the canvas, keep a visible minimum.
      r = max(4.0, self._bound_to_canvas(rad))
      self.canvas.create_oval(cx - r, cy - r, cx + r, cy + r, fill=color, outline="", tags="spot")
      # Keep the index label inside the canvas for spots near the edges.
      lx = min(cx + r + 4, self.CANVAS_SIZE - 16)
      ly = max(cy - r - 4, 14)
      self.canvas.create_text(lx, ly, text=str(i), fill=color, anchor="sw", font=("Helvetica", 10, "bold"), tags="spot")

      if self.var_bound.get():
        self.canvas.create_rectangle(self._bound_to_canvas(left), self._bound_to_canvas(up), self._bound_to_canvas(right), self._bound_to_canvas(down), outline=color, dash=(3, 3), tags="spot")

    self.spot_text.config(state="normal")
    self.spot_text.delete("1.0", "end")
    self.spot_text.insert("end", " #  (   x,   y) Avg Max  R\n")
    self.spot_text.insert("end", "\n".join(lines) if lines else "(no spot)")
    self.spot_text.config(state="disabled")
    self.lbl_count.config(text="Tracked spots: %d" % tracked)

    ## Same log as the console mode, slower than the canvas refresh.
    now = time.time()
    if now - self.last_log >= LOG_INTERVAL_S:
      self.last_log = now
      print_frame()

    ## FPS
    self.frames += 1
    if now - self.fps_t0 >= 1.0:
      self.fps = self.frames / (now - self.fps_t0)
      self.frames = 0
      self.fps_t0 = now
      self.lbl_fps.config(text="FPS: %.1f" % self.fps)

    self.root.after(self.REFRESH_MS, self.refresh)


def run_gui():
  '''!
  @brief Start the tkinter viewer
  @return True if the window was shown, False if tkinter or a display is unavailable
  '''
  if tk is None:
    print("tkinter is not installed, falling back to console mode. (sudo apt install python3-tk)")
    return False
  try:
    root = tk.Tk()
  except Exception as e:
    print("Cannot open a display (%s), falling back to console mode." % e)
    print("Run on the Raspberry Pi desktop or connect with 'ssh -X'.")
    return False
  PositionGUI(root)
  root.mainloop()
  return True


if __name__ == "__main__":
  setup()
  if DISPLAY_MODE == DISPLAY_GUI and run_gui():
    sys.exit(0)
  while True:
    loop()
