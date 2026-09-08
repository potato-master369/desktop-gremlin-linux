// hotspost.c
//   controls hotspots.
//   I put it in a separate file because we are genuinely
//   making main.c too big
#include "animation.h"
#include "trace.h"
#include "config.h"
#include <cairo.h>
#include <gdk/gdk.h>
#include <glib-object.h>
#include <gtk/gtk.h>
#include <stdint.h>
//<Border x:Name="LeftHotspot"
//            Background="red"
//            HorizontalAlignment="Left"
//            Width="40"
//            Margin="40,25,0,0" Height="140" VerticalAlignment="Top"
//            MouseLeftButtonDown="LeftHotspot_Click"/>
//        <Border x:Name="LeftDownHotspot"
//            Background="yellow"
//            HorizontalAlignment="Left"
//            Width="40"
//            MouseLeftButtonDown="LeftDownHotspot_Click" Margin="40,165,0,0"
//            Height="130" VerticalAlignment="Top"/>
//        <Border x:Name="RightHotspot"
//            Background="blue"
//            HorizontalAlignment="Left"
//            Width="39"
//            MouseLeftButtonDown="RightHotspot_Click" Margin="205,25,0,0"
//            Height="145" VerticalAlignment="Top"/>
//        <Border x:Name="RightDownHotspot"
//            Background="Orange"
//            HorizontalAlignment="Left"
//            Width="39"
//            MouseLeftButtonDown="RightDownHotspot_Click" Margin="205,170,0,0"
//            Height="125" VerticalAlignment="Top"
//            RenderTransformOrigin="2.436,0.56"/>
//        <Border x:Name="TopHotspot"
//            Background="Purple"
//            HorizontalAlignment="Left"
//            Width="90"
//            MouseLeftButtonDown="TopHotspot_Click" Margin="97,30,0,0"
//            Height="50" VerticalAlignment="Top"
//            RenderTransformOrigin="0.5,0.5">
//        </Border>

degrli_conf_t *local_conf_hotspot;

// This is a bit janky and stuff but it works so its fine
extern void x_spawn_food(void);
void hotspot_play(int x, int y) {
  if (x >= 40 && x <= 80 && y >= 25 && y <= 165) {
    // LeftHotspot (Red)
    anim_trigger_emote_1();
  } else if (x >= 40 && x <= 80 && y >= 165 && y <= 295) {
    // LeftDownHotspot (Yellow)
    anim_trigger_emote_2();
  } else if (x >= 205 && x <= 244 && y >= 25 && y <= 170) {
    // RightHotspot (Blue)
    anim_trigger_emote_3();
  } else if (x >= 205 && x <= 244 && y >= 170 && y <= 295) {
    // RightDownHotspot (Orange)
    anim_trigger_emote_4();
  } else if (x >= 97 && x <= 187 && y >= 30 && y <= 80) {
    // TopHotspot (Purple)
    x_spawn_food();
  } else {
    // No hotspot hit
    trace_log(TRACE, " [ hotspot] no hotspot hit\n");
  }
}

typedef struct {
  GtkWidget *widget;
  double rel_x;
  double rel_y;
  double width;
  double height;
  double r, g, b, a; // RGBA color components (0.0 to 1.0)
} HotspotBinding;

#define NUM_HOTSPOTS 5
static HotspotBinding hotspots[NUM_HOTSPOTS];

static void draw_hotspot(GtkDrawingArea *drawing_area, cairo_t *cr, int width,
                         int height, gpointer user_data) {
  if (local_conf_hotspot->allow_col_hotspot) {
    HotspotBinding *hb = (HotspotBinding *)user_data;

    cairo_set_source_rgba(cr, hb->r, hb->g, hb->b, hb->a);
    cairo_paint(cr);
  }
}

void hotspot_init(GtkFixed *fcontainer, double sprite_x, double sprite_y) {
  double data[NUM_HOTSPOTS][8] = {
      {40.0, 25.0, 40.0, 140.0, 1.0, 0.0, 0.0, 1.0}, // LeftHotspot (Red)
      {40.0, 165.0, 40.0, 130.0, 1.0, 1.0, 0.0,
       1.0}, // LeftDownHotspot (Yellow)
      {205.0, 25.0, 39.0, 145.0, 0.0, 0.0, 1.0, 1.0}, // RightHotspot (Blue)
      {205.0, 170.0, 39.0, 125.0, 1.0, 0.5, 1.0,
       1.0},                                       // RightDownHotspot (Orange)
      {97.0, 30.0, 90.0, 50.0, 0.5, 0.0, 0.5, 1.0} // TopHotspot (Purple)
  };

  for (int i = 0; i < NUM_HOTSPOTS; i++) {
    hotspots[i].rel_x = data[i][0];
    hotspots[i].rel_y = data[i][1];
    hotspots[i].width = data[i][2];
    hotspots[i].height = data[i][3];
    hotspots[i].r = data[i][4];
    hotspots[i].g = data[i][5];
    hotspots[i].b = data[i][6];
    hotspots[i].a = data[i][7];

    hotspots[i].widget = gtk_drawing_area_new();
    gtk_widget_set_size_request(hotspots[i].widget, (int)hotspots[i].width,
                                (int)hotspots[i].height);

    gtk_drawing_area_set_draw_func(GTK_DRAWING_AREA(hotspots[i].widget),
                                   draw_hotspot, &hotspots[i], NULL);


    double abs_x = sprite_x + hotspots[i].rel_x;
    double abs_y = sprite_y + hotspots[i].rel_y;

    gtk_fixed_put(fcontainer, hotspots[i].widget, abs_x, abs_y);
  }

  local_conf_hotspot = degrli_request_localconf();
}

GtkWidget *hotspot_get_hotspot(int n) {
  return n < NUM_HOTSPOTS ? hotspots[n].widget : NULL;
}

void hotspot_update(GtkFixed *fcontainer, double sprite_x, double sprite_y) {
  trace_log(DEBUG, " [ hotspot] Updating hotspots sprite_x: %d, sprite_y: %d\n",
            sprite_x, sprite_y);
  for (int i = 0; i < NUM_HOTSPOTS; i++) {
    double new_x = sprite_x + hotspots[i].rel_x;
    double new_y = sprite_y + hotspots[i].rel_y;

    gtk_fixed_move(fcontainer, hotspots[i].widget, new_x, new_y);
  }
}
