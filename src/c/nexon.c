// Nexon -- analoges Watchface fuer Emery und Gabbro.
//
// Auf Emery kissenfoermig, auf Gabbro rund: Zifferblatt im Stil der 70er: aeusserer Kranz mit
// applizierten Stabindizes, innenliegender quadratischer Minutenring,
// Schriftzug oben, Datumsfenster bei 6 Uhr. Drei Farbschemata (Tuerkis,
// Weiss mit Marine-Ring, Marine) und ein abschaltbarer Sekundenzeiger.
// canvas_update ist der einzige Zeichen-Einstiegspunkt.
//
// Alle Positionen laufen ueber Strahlen vom Mittelpunkt: polar() liefert den
// Punkt in Abstand r und seitlichem Versatz off, rect_r() den Abstand, in dem
// ein Strahl ein achsparalleles Rechteck trifft. So sitzen Indizes und
// Minutenstriche auf Rechtecken statt auf Kreisen. Polygone werden direkt
// gerechnet, gpath_rotate_to wird bewusst nicht benutzt.

#include <pebble.h>

// ---- Layout ----------------------------------------------------------------
#ifdef PBL_PLATFORM_EMERY
#define IDX_HW         90     // Aussenkante der Indizes, halbe Breite/Hoehe
#define IDX_HH         104
#define IDX_LEN        19
#define IDX_HALF_W     3
#define RING_O_HW      76     // Minutenring aussen
#define RING_O_HH      90
#define RING_I_HW      67     // Minutenring innen
#define RING_I_HH      81
#define RING_RADIUS    3
#define HOUR_LEN       50
#define HOUR_HALF_W    5
#define MIN_LEN        80
#define MIN_HALF_W     4
#define SEC_LEN        84
#define SEC_TAIL       18
#define HAND_TAIL      10
#define CAP_R          4
#define LOGO_Y         -60    // relativ zur Mitte
#define LOGO_FONT      FONT_KEY_GOTHIC_18_BOLD
#define LOGO_H         22
#define SUB_FONT       FONT_KEY_GOTHIC_09
#define SUB_H          12
#define DATE_FONT      FONT_KEY_GOTHIC_14_BOLD
#define DATE_W         16
#define DATE_H         17
#define DATE_GAP       3      // Abstand Datumsfenster unter dem Ring
#define DATE_TEXT_DY   -2
#define FRAME_W        3
#else
// Rund: alle "Rechtecke" sind Kreise, HW = HH = Radius.
#define IDX_HW         117
#define IDX_HH         117
#define IDX_LEN        21
#define IDX_HALF_W     3
#define RING_O_HW      94
#define RING_O_HH      94
#define RING_I_HW      83
#define RING_I_HH      83
#define RING_RADIUS    3
#define HOUR_LEN       70
#define HOUR_HALF_W    6
#define MIN_LEN        110
#define MIN_HALF_W     5
#define SEC_LEN        116
#define SEC_TAIL       24
#define HAND_TAIL      11
#define CAP_R          5
#define LOGO_Y         -58
#define LOGO_FONT      FONT_KEY_GOTHIC_24_BOLD
#define LOGO_H         28
#define SUB_FONT       FONT_KEY_GOTHIC_14
#define SUB_H          16
#define DATE_FONT      FONT_KEY_GOTHIC_18_BOLD
#define DATE_W         20
#define DATE_H         21
#define DATE_GAP       5
#define DATE_TEXT_DY   -3
#define FRAME_W        4
#endif

#define BRAND_TEXT     "NEXON"
#define SUB_TEXT       "KLARWERK"

// ---- Farbschemata ----------------------------------------------------------
typedef struct {
  GColor dial;        // Zifferblatt
  GColor ring;        // Flaeche des Minutenrings
  GColor ring_line;   // Kanten des Minutenrings
  GColor tick;        // Minutenstriche
  GColor idx;         // Indizes (Flaeche)
  GColor idx_edge;    // Indizes (Kante)
  GColor idx_hi;      // Indizes (Glanzlinie)
  GColor hand;        // Zeiger (Flaeche)
  GColor hand_edge;   // Zeiger (Kante)
  GColor lume;        // Leuchtstreifen in den Zeigern
  GColor sec;         // Sekundenzeiger
  GColor logo;        // Schriftzug
  GColor sub;         // Unterzeile
  GColor shadow;      // Schlagschatten der Schrift, = dial heisst keiner
  GColor date_bg;
  GColor date_fg;
  GColor frame;       // Gehaeusekante am Displayrand
} Scheme;

#define SCHEME_COUNT 3

static Scheme s_schemes[SCHEME_COUNT];

static void schemes_init(void) {
  // 0: Tuerkis -- helles Blatt, silberne Zeiger und Indizes
  s_schemes[0] = (Scheme) {
    .dial = GColorCyan,           .ring = GColorCyan,
    .ring_line = GColorWhite,     .tick = GColorWhite,
    .idx = GColorLightGray,       .idx_edge = GColorMidnightGreen, .idx_hi = GColorWhite,
    .hand = GColorLightGray,      .hand_edge = GColorMidnightGreen, .lume = GColorWhite,
    .sec = GColorWhite,
    .logo = GColorWhite,          .sub = GColorWhite,      .shadow = GColorMidnightGreen,
    .date_bg = GColorWhite,       .date_fg = GColorBlack,
    .frame = GColorLightGray,
  };
  // 1: Weiss -- Marine-Minutenring, dunkle Zeiger mit Leuchtstreifen
  s_schemes[1] = (Scheme) {
    .dial = GColorWhite,          .ring = GColorOxfordBlue,
    .ring_line = GColorOxfordBlue, .tick = GColorWhite,
    .idx = GColorLightGray,       .idx_edge = GColorDarkGray, .idx_hi = GColorWhite,
    .hand = GColorOxfordBlue,     .hand_edge = GColorOxfordBlue, .lume = GColorWhite,
    .sec = GColorOxfordBlue,
    .logo = GColorBlack,          .sub = GColorDarkGray,   .shadow = GColorWhite,
    .date_bg = GColorBlack,       .date_fg = GColorWhite,
    .frame = GColorLightGray,
  };
  // 2: Marine -- dunkles Blatt, weisse Striche, silberne Zeiger
  s_schemes[2] = (Scheme) {
    .dial = GColorOxfordBlue,     .ring = GColorOxfordBlue,
    .ring_line = GColorLightGray, .tick = GColorWhite,
    .idx = GColorLightGray,       .idx_edge = GColorDarkGray, .idx_hi = GColorWhite,
    .hand = GColorLightGray,      .hand_edge = GColorDarkGray, .lume = GColorWhite,
    .sec = GColorWhite,
    .logo = GColorWhite,          .sub = GColorLightGray,  .shadow = GColorOxfordBlue,
    .date_bg = GColorWhite,       .date_fg = GColorBlack,
    .frame = GColorLightGray,
  };
}

// ---- AppMessage ------------------------------------------------------------
// Faustformel: 1 + 7 je Key + Nutzdaten; Clay schickt die Auswahl als String.
#define INBOX_SIZE     128
#define OUTBOX_SIZE    64

// ---- Einstellungen ---------------------------------------------------------
// Byte-Dump: neue Felder NUR am Ende anhaengen.
#define SETTINGS_KEY      1
#define SETTINGS_VERSION  1

typedef struct {
  uint8_t version;
  uint8_t scheme;
  bool    seconds;
} Settings;

static Window *s_window;
static Layer *s_canvas;
static GFont s_logo_font, s_sub_font, s_date_font;
static Settings s_settings;
static GPoint s_center;

// Puffer fuer Polygone, static wegen des knappen App-Stacks.
#define POLY_MAX 6
static GPoint s_poly[POLY_MAX];

static void settings_defaults(void) {
  s_settings.version = SETTINGS_VERSION;
  s_settings.scheme = 0;
  s_settings.seconds = true;
}

static void settings_load(void) {
  settings_defaults();
  if (persist_exists(SETTINGS_KEY)) {
    persist_read_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
  }
  if (s_settings.scheme >= SCHEME_COUNT) {
    s_settings.scheme = 0;
  }
}

static void settings_save(void) {
  s_settings.version = SETTINGS_VERSION;
  persist_write_data(SETTINGS_KEY, &s_settings, sizeof(s_settings));
}

// ---- Geometrie -------------------------------------------------------------

// Punkt auf dem Strahl unter Winkel a (0 = 12 Uhr, im Uhrzeigersinn) im
// Abstand r, seitlich um off versetzt (positiv = im Uhrzeigersinn).
static GPoint polar(int32_t a, int32_t r, int32_t off) {
  int32_t s = sin_lookup(a);
  int32_t c = cos_lookup(a);
  return GPoint(s_center.x + (r * s - off * c) / TRIG_MAX_RATIO,
                s_center.y + (-r * c - off * s) / TRIG_MAX_RATIO);
}

// Abstand vom Mittelpunkt, in dem der Strahl das Rechteck hw x hh trifft.
// Auf Gabbro ist die Form ein Kreis mit Radius hw.
static int32_t rect_r(int32_t a, int32_t hw, int32_t hh) {
#ifndef PBL_PLATFORM_EMERY
  return hw;
#endif
  int32_t s = abs(sin_lookup(a));
  int32_t c = abs(cos_lookup(a));
  int32_t rx = s > 0 ? hw * TRIG_MAX_RATIO / s : 10000;
  int32_t ry = c > 0 ? hh * TRIG_MAX_RATIO / c : 10000;
  return rx < ry ? rx : ry;
}

static void fill_poly(GContext *ctx, int n, GColor fill, GColor edge) {
  GPath path = { .num_points = n, .points = s_poly, .rotation = 0, .offset = GPointZero };
  graphics_context_set_fill_color(ctx, fill);
  gpath_draw_filled(ctx, &path);
  graphics_context_set_stroke_color(ctx, edge);
  graphics_context_set_stroke_width(ctx, 1);
  gpath_draw_outline(ctx, &path);
}

// Rechteckiger Balken entlang des Strahls von r0 bis r1, halbe Breite w.
static void bar_poly(int32_t a, int32_t r0, int32_t r1, int32_t w) {
  s_poly[0] = polar(a, r0, -w);
  s_poly[1] = polar(a, r1, -w);
  s_poly[2] = polar(a, r1, w);
  s_poly[3] = polar(a, r0, w);
}

// Zeiger: spitz zulaufender Stab mit kurzem Gegenstueck.
static void hand_poly(int32_t a, int32_t len, int32_t tail, int32_t w) {
  s_poly[0] = polar(a, -tail, -w);
  s_poly[1] = polar(a, len - 3 * w, -w);
  s_poly[2] = polar(a, len, 0);
  s_poly[3] = polar(a, len - 3 * w, w);
  s_poly[4] = polar(a, -tail, w);
}

// ---- Zeichnen --------------------------------------------------------------

#ifdef PBL_PLATFORM_EMERY
static GRect centered_rect(int16_t hw, int16_t hh) {
  return GRect(s_center.x - hw, s_center.y - hh, 2 * hw + 1, 2 * hh + 1);
}
#endif

static void draw_frame(GContext *ctx, GRect b, const Scheme *sc) {
  graphics_context_set_stroke_color(ctx, sc->frame);
  graphics_context_set_stroke_width(ctx, FRAME_W);
#ifdef PBL_PLATFORM_EMERY
  graphics_draw_round_rect(ctx, grect_inset(b, GEdgeInsets(FRAME_W / 2)), 8);
#else
  graphics_draw_circle(ctx, s_center, b.size.w / 2 - FRAME_W / 2);
#endif
}

static void draw_ring(GContext *ctx, const Scheme *sc) {
#ifdef PBL_PLATFORM_EMERY
  GRect outer = centered_rect(RING_O_HW, RING_O_HH);
  GRect inner = centered_rect(RING_I_HW, RING_I_HH);
  graphics_context_set_fill_color(ctx, sc->ring);
  graphics_fill_rect(ctx, outer, RING_RADIUS, GCornersAll);
  graphics_context_set_fill_color(ctx, sc->dial);
  graphics_fill_rect(ctx, inner, RING_RADIUS, GCornersAll);

  graphics_context_set_stroke_color(ctx, sc->ring_line);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_round_rect(ctx, outer, RING_RADIUS);
  graphics_draw_round_rect(ctx, inner, RING_RADIUS);
#else
  graphics_context_set_fill_color(ctx, sc->ring);
  graphics_fill_circle(ctx, s_center, RING_O_HW);
  graphics_context_set_fill_color(ctx, sc->dial);
  graphics_fill_circle(ctx, s_center, RING_I_HW);

  graphics_context_set_stroke_color(ctx, sc->ring_line);
  graphics_context_set_stroke_width(ctx, 1);
  graphics_draw_circle(ctx, s_center, RING_O_HW);
  graphics_draw_circle(ctx, s_center, RING_I_HW);
#endif

  // Minutenstriche quer ueber den Ring, zur vollen Stunde ueber die ganze
  // Breite, sonst nur die aeussere Haelfte.
  graphics_context_set_stroke_color(ctx, sc->tick);
  for (int m = 0; m < 60; m++) {
    int32_t a = TRIG_MAX_ANGLE * m / 60;
    int32_t ro = rect_r(a, RING_O_HW, RING_O_HH) - 1;
    int32_t ri = rect_r(a, RING_I_HW, RING_I_HH) + 1;
    bool hour = (m % 5) == 0;
    if (!hour) {
      ri = (ro + ri) / 2;
    }
    graphics_context_set_stroke_width(ctx, hour ? 2 : 1);
    graphics_draw_line(ctx, polar(a, ri, 0), polar(a, ro, 0));
  }
}

static void draw_indices(GContext *ctx, const Scheme *sc) {
  for (int h = 0; h < 12; h++) {
    if (h == 6) {
      continue;   // dort sitzt das Datum
    }
    int32_t a = TRIG_MAX_ANGLE * h / 12;
    int32_t ro = rect_r(a, IDX_HW, IDX_HH);
    int32_t ri = ro - IDX_LEN;
    bar_poly(a, ri, ro, IDX_HALF_W);
    fill_poly(ctx, 4, sc->idx, sc->idx_edge);
    // Glanzlinie in der Mitte des Balkens
    graphics_context_set_stroke_color(ctx, sc->idx_hi);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, polar(a, ri + 2, 0), polar(a, ro - 2, 0));
  }
}

static void draw_texts(GContext *ctx, const Scheme *sc, struct tm *t) {
  int16_t w = 2 * RING_I_HW;
  int16_t x = s_center.x - RING_I_HW;
  int16_t y = s_center.y + LOGO_Y;

  // Erst der Schatten (1 px nach rechts unten), dann die Schrift.
  bool shadow = !gcolor_equal(sc->shadow, sc->dial);
  for (int pass = shadow ? 0 : 1; pass < 2; pass++) {
    int16_t d = pass == 0 ? 1 : 0;
    graphics_context_set_text_color(ctx, pass == 0 ? sc->shadow : sc->logo);
    graphics_draw_text(ctx, BRAND_TEXT, s_logo_font, GRect(x + d, y + d, w, LOGO_H),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
    graphics_context_set_text_color(ctx, pass == 0 ? sc->shadow : sc->sub);
    graphics_draw_text(ctx, SUB_TEXT, s_sub_font, GRect(x + d, y + LOGO_H - 4 + d, w, SUB_H),
                       GTextOverflowModeFill, GTextAlignmentCenter, NULL);
  }

  // Datumsfenster unter dem Ring anstelle des 6-Uhr-Index, silbern gefasst.
  static char day[4];
  snprintf(day, sizeof(day), "%d", t->tm_mday);
  GRect win = GRect(s_center.x - DATE_W / 2, s_center.y + RING_O_HH + DATE_GAP,
                    DATE_W, DATE_H);
  graphics_context_set_fill_color(ctx, sc->idx);
  graphics_fill_rect(ctx, grect_inset(win, GEdgeInsets(-2)), 2, GCornersAll);
  graphics_context_set_fill_color(ctx, sc->date_bg);
  graphics_fill_rect(ctx, win, 0, GCornerNone);
  graphics_context_set_text_color(ctx, sc->date_fg);
  graphics_draw_text(ctx, day, s_date_font,
                     GRect(win.origin.x - 4, win.origin.y + DATE_TEXT_DY, win.size.w + 8, win.size.h + 6),
                     GTextOverflowModeFill, GTextAlignmentCenter, NULL);
}

static void draw_hand(GContext *ctx, const Scheme *sc, int32_t a, int32_t len, int32_t w) {
  hand_poly(a, len, HAND_TAIL, w);
  fill_poly(ctx, 5, sc->hand, sc->hand_edge);
  // Leuchtstreifen: schmalerer Stab im Inneren
  int32_t lw = w - 2;
  if (lw >= 1) {
    bar_poly(a, CAP_R + 4, len - 3 * w, lw);
    fill_poly(ctx, 4, sc->lume, sc->lume);
  }
}

static void canvas_update(Layer *layer, GContext *ctx) {
  GRect b = layer_get_bounds(layer);
  const Scheme *sc = &s_schemes[s_settings.scheme];
  s_center = grect_center_point(&b);
  graphics_context_set_antialiased(ctx, true);

  graphics_context_set_fill_color(ctx, sc->dial);
  graphics_fill_rect(ctx, b, 0, GCornerNone);

  time_t now = time(NULL);
  struct tm *t = localtime(&now);

  draw_ring(ctx, sc);
  draw_indices(ctx, sc);
  draw_texts(ctx, sc, t);
  draw_frame(ctx, b, sc);

  int32_t min_a = TRIG_MAX_ANGLE * (t->tm_min * 60 + t->tm_sec) / 3600;
  int32_t hour_a = TRIG_MAX_ANGLE * ((t->tm_hour % 12) * 60 + t->tm_min) / 720;
  draw_hand(ctx, sc, hour_a, HOUR_LEN, HOUR_HALF_W);
  draw_hand(ctx, sc, min_a, MIN_LEN, MIN_HALF_W);

  GColor cap = sc->hand;
  if (s_settings.seconds) {
    int32_t sec_a = TRIG_MAX_ANGLE * t->tm_sec / 60;
    graphics_context_set_stroke_color(ctx, sc->sec);
    graphics_context_set_stroke_width(ctx, 1);
    graphics_draw_line(ctx, polar(sec_a, -SEC_TAIL, 0), polar(sec_a, SEC_LEN, 0));
    cap = sc->sec;
  }
  graphics_context_set_fill_color(ctx, cap);
  graphics_fill_circle(ctx, s_center, CAP_R);
  graphics_context_set_fill_color(ctx, sc->hand_edge);
  graphics_fill_circle(ctx, s_center, 1);
}

// ---- Ereignisse ------------------------------------------------------------

static void tick_handler(struct tm *tick_time, TimeUnits changed) {
  layer_mark_dirty(s_canvas);
}

static void subscribe_ticks(void) {
  tick_timer_service_unsubscribe();
  tick_timer_service_subscribe(s_settings.seconds ? SECOND_UNIT : MINUTE_UNIT, tick_handler);
}

static int32_t tuple_int(Tuple *t) {
  if (t->type == TUPLE_CSTRING) {
    return atoi(t->value->cstring);
  }
  return t->value->int32;
}

static void inbox_received(DictionaryIterator *iter, void *context) {
  Tuple *t;
  if ((t = dict_find(iter, MESSAGE_KEY_SCHEME))) {
    s_settings.scheme = (uint8_t)tuple_int(t);
  }
  if ((t = dict_find(iter, MESSAGE_KEY_SECONDS))) {
    s_settings.seconds = tuple_int(t) != 0;
  }
  settings_save();
  settings_load();
  subscribe_ticks();
  layer_mark_dirty(s_canvas);
}

static void window_load(Window *window) {
  Layer *root = window_get_root_layer(window);
  s_canvas = layer_create(layer_get_bounds(root));
  layer_set_update_proc(s_canvas, canvas_update);
  layer_add_child(root, s_canvas);
}

static void window_unload(Window *window) {
  layer_destroy(s_canvas);
}

static void init(void) {
  schemes_init();
  settings_load();
  s_logo_font = fonts_get_system_font(LOGO_FONT);
  s_sub_font = fonts_get_system_font(SUB_FONT);
  s_date_font = fonts_get_system_font(DATE_FONT);

  s_window = window_create();
  window_set_window_handlers(s_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  window_stack_push(s_window, true);

  subscribe_ticks();

  app_message_register_inbox_received(inbox_received);
  AppMessageResult r = app_message_open(INBOX_SIZE, OUTBOX_SIZE);
  if (r != APP_MSG_OK) {
    APP_LOG(APP_LOG_LEVEL_ERROR, "app_message_open: %d", (int)r);
  }
}

static void deinit(void) {
  tick_timer_service_unsubscribe();
  window_destroy(s_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
