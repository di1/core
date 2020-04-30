#include <gui/gui.h>

GtkListStore* store = NULL;

static void activate(GtkApplication* app, gpointer user_data) {
  (void)user_data;

  /*
   * Create an application
   */
  GtkWidget* window;
  window = gtk_application_window_new(app);
  gtk_window_set_title(GTK_WINDOW(window), "Riski");

  /*
   * Set application sizes
   */
  gtk_window_set_resizable(GTK_WINDOW(window), true);
  gtk_window_set_default_size(GTK_WINDOW(window), 1024, 720);

  /*
   * Create the layout
   * |=========|========|
   * |         |        |
   * |         |        |
   * |         |        |
   * |=========|========|
   * |         |        |
   * |         |        |
   * |         |        |
   * |=========|========|
   */
  GtkWidget* grid_layout;
  grid_layout = gtk_grid_new();

  gtk_grid_insert_row(GTK_GRID(grid_layout), 0);
  gtk_grid_insert_row(GTK_GRID(grid_layout), 1);

  gtk_grid_insert_column(GTK_GRID(grid_layout), 0);
  gtk_grid_insert_column(GTK_GRID(grid_layout), 1);

  gtk_grid_set_row_homogeneous(GTK_GRID(grid_layout), true);
  gtk_grid_set_column_homogeneous(GTK_GRID(grid_layout), true);

  /*
   * Create a list store for ticker information
   */
  store = gtk_list_store_new(PAC_NUM, G_TYPE_STRING, G_TYPE_STRING,
                             G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);

  GtkWidget* tree;
  tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));

  g_object_unref(G_OBJECT(store));

  GtkTreeViewColumn* column;
  GtkCellRenderer* renderer;

  /*
   * Create the columns
   */
  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("SEC", renderer, "text",
                                                    PAC_SEC, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("BID", renderer, "text",
                                                    PAC_BID, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("ASK", renderer, "text",
                                                    PAC_ASK, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("LST", renderer, "text",
                                                    PAC_LST, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  renderer = gtk_cell_renderer_text_new();
  column = gtk_tree_view_column_new_with_attributes("CHG", renderer, "text",
                                                    PAC_CHG, NULL);
  gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);

  /*
   * Insert the treeview into a scroll view
   */
  GtkWidget* sw = NULL;
  sw = gtk_scrolled_window_new(NULL, NULL);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER,
                                 GTK_POLICY_AUTOMATIC);
  gtk_scrolled_window_set_shadow_type(GTK_SCROLLED_WINDOW(sw),
                                      GTK_SHADOW_ETCHED_IN);
  gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(sw), GTK_POLICY_NEVER,
                                 GTK_POLICY_AUTOMATIC);
  gtk_container_add(GTK_CONTAINER(sw), tree);

  /*
   * Add the listview to the bottom left
   */
  gtk_grid_attach(GTK_GRID(grid_layout), sw, 0, 1, 1, 1);

  GtkWidget* chart_area = gtk_drawing_area_new();
  gtk_grid_attach(GTK_GRID(grid_layout), chart_area, 1, 0, 2, 2);

  gtk_container_add(GTK_CONTAINER(window), grid_layout);

  gtk_widget_show_all(window);
}

void gui_start(int argc, char** argv) {
  GtkApplication* app;
  int status;

  app = gtk_application_new("io.riski", G_APPLICATION_FLAGS_NONE);
  g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
  status = g_application_run(G_APPLICATION(app), argc, argv);
  g_object_unref(app);

  (void)status;
}
