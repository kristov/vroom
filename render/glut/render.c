#include <gtk/gtk.h>

typedef struct app {
    GtkWidget* card_manager;
    GtkWidget* card_manager_view;
} app_t;

/*
static void delete_clicked(GtkButton *btn, gpointer data) {
    gtk_button_set_label(btn, "Hello World");
}

static void display_clicked(GtkButton *btn, gpointer data) {
    gtk_button_set_label(btn, "Hello World");
}
*/

/*
selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
    gchar *name;

    gtk_tree_model_get (model, &iter, COL_NAME, &name, -1);

    g_print ("selected row is: %s\n", name);

    g_free(name);
}
*/

void view_popup_menu_onDoSomething (GtkWidget *menuitem, gpointer userdata) {
    //GtkTreeView *treeview = GTK_TREE_VIEW(userdata);
    g_print("Do something!\n");
}

void screen_popup_menu(GtkWidget *treeview, gpointer userdata) {
    GtkWidget* menu = gtk_menu_new();

    GtkWidget* unplug = gtk_menu_item_new_with_label("Unplug screen");
    g_signal_connect(unplug, "activate", (GCallback)view_popup_menu_onDoSomething, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), unplug);

    gtk_widget_show_all(menu);
    gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

void card_popup_menu(GtkWidget *treeview, gpointer userdata) {
    GtkWidget* menu = gtk_menu_new();

    GtkWidget* disconnect = gtk_menu_item_new_with_label("Remove card");
    g_signal_connect(disconnect, "activate", (GCallback)view_popup_menu_onDoSomething, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), disconnect);

    GtkWidget* add_screen = gtk_menu_item_new_with_label("Add screen");
    g_signal_connect(add_screen, "activate", (GCallback)view_popup_menu_onDoSomething, treeview);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), add_screen);

    gtk_widget_show_all(menu);
    gtk_menu_popup_at_pointer(GTK_MENU(menu), NULL);
}

gboolean on_button_pressed(GtkWidget *treeview, GdkEventButton *event, gpointer userdata) {
    //app_t* app = (app_t*)userdata;
    if (event->type != GDK_BUTTON_PRESS || event->button != 3) {
        return FALSE;
    }

    g_print("Single right click on the tree view.\n");

    GtkTreeSelection* selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));

    if (gtk_tree_selection_count_selected_rows(selection) <= 1) {
        GtkTreePath *path;
        if (gtk_tree_view_get_path_at_pos(GTK_TREE_VIEW(treeview), (gint)event->x, (gint)event->y, &path, NULL, NULL, NULL)) {
            gtk_tree_selection_unselect_all(selection);
            gtk_tree_selection_select_path(selection, path);
            gtk_tree_path_free(path);
        }
        GtkTreeIter row;
        GtkTreeModel* model;
        gchar* item = NULL;
        gtk_tree_selection_get_selected(selection, &model, &row);
        gtk_tree_model_get(model, &row, 1, &item, -1);
        if (item == NULL) {
            gtk_tree_model_get(model, &row, 3, &item, -1);
            screen_popup_menu(treeview, NULL);
        }
        else {
            card_popup_menu(treeview, NULL);
        }
        g_print("%s\n", item);
        g_free(item);
    }

    return TRUE;
}

void insert_screen(app_t* app, const char* name) {
    GtkTreeIter toplevel, row;
    GtkTreeStore* model = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(app->card_manager_view)));
    gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(model), &toplevel, NULL, 0);
    gtk_tree_store_append(model, &row, &toplevel);
    gtk_tree_store_set(model, &row, 2, "display", 3, name, -1);
}

void insert_card(app_t* app, const char* device) {
    GtkTreeStore* model = GTK_TREE_STORE(gtk_tree_view_get_model(GTK_TREE_VIEW(app->card_manager_view)));
    GtkTreeIter row;
    gtk_tree_store_append(model, &row, NULL);
    gtk_tree_store_set(model, &row, 0, "vcard", 1, device, -1);
    //gtk_tree_store_set(model, &toplevel, 0, "vcard", 1, "/dev/dri/card0", 2, "display", 3, "LVDS 1024x768", -1);
}

void create_view(app_t* app) {
    GtkTreeViewColumn* card_name = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(card_name, "DRI card");

    GtkCellRenderer* card_icon_renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(card_name, card_icon_renderer, FALSE);
    gtk_tree_view_column_add_attribute(card_name, card_icon_renderer, "icon-name", 0);

    GtkCellRenderer* card_name_renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(card_name, card_name_renderer, FALSE);
    gtk_tree_view_column_add_attribute(card_name, card_name_renderer, "text", 1);

    gtk_tree_view_append_column(GTK_TREE_VIEW(app->card_manager_view), card_name);

    GtkTreeViewColumn* screen_name = gtk_tree_view_column_new();
    gtk_tree_view_column_set_title(screen_name, "Screen");

    GtkCellRenderer* screen_icon_renderer = gtk_cell_renderer_pixbuf_new();
    gtk_tree_view_column_pack_start(screen_name, screen_icon_renderer, FALSE);
    gtk_tree_view_column_add_attribute(screen_name, screen_icon_renderer, "icon-name", 2);

    GtkCellRenderer* screen_name_renderer = gtk_cell_renderer_text_new();
    gtk_tree_view_column_pack_start(screen_name, screen_name_renderer, FALSE);
    gtk_tree_view_column_add_attribute(screen_name, screen_name_renderer, "text", 3);

    gtk_tree_view_append_column(GTK_TREE_VIEW(app->card_manager_view), screen_name);

    GtkTreeStore* model = gtk_tree_store_new(4, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
    gtk_tree_view_set_model(GTK_TREE_VIEW(app->card_manager_view), GTK_TREE_MODEL(model));
}

int main(int argc, char **argv) {
    app_t app;
    gtk_init(&argc, &argv);

    GtkBuilder* builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "render.glade", NULL);

    app.card_manager = GTK_WIDGET(gtk_builder_get_object(builder, "card_manager"));
    g_signal_connect(app.card_manager, "delete_event", G_CALLBACK(gtk_main_quit), NULL);

    app.card_manager_view = GTK_WIDGET(gtk_builder_get_object(builder, "card_manager_view"));
    g_signal_connect(app.card_manager_view, "button-press-event", (GCallback)on_button_pressed, NULL);
    create_view(&app);
    insert_card(&app, "/dev/dri/card0");
    insert_card(&app, "/dev/dri/card1");
    insert_screen(&app, "LVDS 1024x768");

    g_object_unref(G_OBJECT(builder));

    gtk_widget_show_all(app.card_manager);
    gtk_main();

    return 0;
}
