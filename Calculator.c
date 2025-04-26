#include <gtk/gtk.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

GtkWidget *entry;
GtkWidget *label_result;

#define MAX_HISTORY 100
char *history[MAX_HISTORY];
int history_index = 0;

// دالة إضافة العمليات إلى الذاكرة مع الحذف لما تتملي
void append_to_history(const char *expression, double result) {
    char buffer[128];
    snprintf(buffer, sizeof(buffer), "%s = %.2f", expression, result);

    if (history_index >= MAX_HISTORY) {
        g_free(history[0]);
        for (int i = 1; i < MAX_HISTORY; i++) {
            history[i-1] = history[i];
        }
        history_index = MAX_HISTORY - 1;
    }

    history[history_index++] = g_strdup(buffer);
}

// دالة تحرير الذاكرة عند الإغلاق
void free_history() {
    for (int i = 0; i < history_index; i++) {
        g_free(history[i]);
    }
}

// دالة الحساب البسيط (من غير مكتبات ولا أقواس)
double evaluate_expression(const char *expr) {
    double result = 0.0;
    double current = 0.0;
    char op = '+';

    for (int i = 0; expr[i] != '\0'; i++) {
        if (expr[i] >= '0' && expr[i] <= '9' || expr[i] == '.') {
            char number[32];
            int j = 0;

            while ((expr[i] >= '0' && expr[i] <= '9') || expr[i] == '.') {
                number[j++] = expr[i++];
            }
            number[j] = '\0';
            current = atof(number);
            i--;

            if (op == '+')
                result += current;
            else if (op == '-')
                result -= current;
            else if (op == '*')
                result *= current;
            else if (op == '/')
                result /= current;
        } else if (expr[i] == '+' || expr[i] == '-' || expr[i] == '*' || expr[i] == '/') {
            op = expr[i];
        }
    }
    return result;
}

// لما يضغط رقم أو رمز
void on_button_clicked(GtkButton *button, gpointer user_data) {
    const gchar *value = (const gchar *)user_data;
    const gchar *current = gtk_editable_get_text(GTK_EDITABLE(entry));
    gchar *new_text = g_strconcat(current, value, NULL);
    gtk_editable_set_text(GTK_EDITABLE(entry), new_text);
    g_free(new_text);
}

// لما يضغط يساوي =
void on_equal_clicked(GtkButton *button, gpointer user_data) {
    const gchar *expression = gtk_editable_get_text(GTK_EDITABLE(entry));
    double result = evaluate_expression(expression);

    append_to_history(expression, result);

    gchar result_str[64];
    snprintf(result_str, sizeof(result_str), "%.2f", result);
    gtk_label_set_text(GTK_LABEL(label_result), result_str);
}

// لما يضغط مسح الكل
void on_clear_clicked(GtkButton *button, gpointer user_data) {
    gtk_editable_set_text(GTK_EDITABLE(entry), "");
    gtk_label_set_text(GTK_LABEL(label_result), "");
}

// لما يضغط باك سبيس
void on_backspace_clicked(GtkButton *button, gpointer user_data) {
    const gchar *current = gtk_editable_get_text(GTK_EDITABLE(entry));
    int len = strlen(current);
    if (len > 0) {
        gchar *new_text = g_strndup(current, len - 1);
        gtk_editable_set_text(GTK_EDITABLE(entry), new_text);
        g_free(new_text);
    }
}

// عرض تاريخ العمليات في نافذة جديدة
void show_history(GtkButton *button, gpointer user_data) {
    GtkWidget *history_window = gtk_window_new();
    gtk_window_set_title(GTK_WINDOW(history_window), "History");
    gtk_window_set_default_size(GTK_WINDOW(history_window), 300, 400);

    GtkWidget *scrolled_window = gtk_scrolled_window_new();
    gtk_window_set_child(GTK_WINDOW(history_window), scrolled_window);

    GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_scrolled_window_set_child(GTK_SCROLLED_WINDOW(scrolled_window), box);

    for (int i = 0; i < history_index; i++) {
        GtkWidget *label = gtk_label_new(history[i]);
        gtk_box_append(GTK_BOX(box), label);
    }

    gtk_window_present(GTK_WINDOW(history_window));
}

// واجهة البرنامج الأساسية
static void activate(GtkApplication *app, gpointer user_data) {
    GtkWidget *window = gtk_application_window_new(app);
    gtk_window_set_title(GTK_WINDOW(window), "Calculator");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 250);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

    GtkWidget *main_box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    gtk_widget_set_margin_top(main_box, 20);
    gtk_widget_set_margin_bottom(main_box, 20);
    gtk_widget_set_margin_start(main_box, 20);
    gtk_widget_set_margin_end(main_box, 20);
    gtk_window_set_child(GTK_WINDOW(window), main_box);

    entry = gtk_entry_new();
    gtk_widget_set_margin_bottom(entry, 10);
    gtk_box_append(GTK_BOX(main_box), entry);

    label_result = gtk_label_new("");
    gtk_widget_set_margin_bottom(label_result, 10);
    gtk_box_append(GTK_BOX(main_box), label_result);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_box_append(GTK_BOX(main_box), grid);

    const gchar *buttons[4][4] = {
        {"7", "8", "9", "/"},
        {"4", "5", "6", "*"},
        {"1", "2", "3", "-"},
        {"0", ".", "=", "+"}
    };

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            GtkWidget *btn = gtk_button_new_with_label(buttons[i][j]);
            gtk_grid_attach(GTK_GRID(grid), btn, j, i, 1, 1);

            if (g_strcmp0(buttons[i][j], "=") == 0) {
                g_signal_connect(btn, "clicked", G_CALLBACK(on_equal_clicked), NULL);
            } else {
                g_signal_connect(btn, "clicked", G_CALLBACK(on_button_clicked), (gpointer)buttons[i][j]);
            }
        }
    }

    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    gtk_widget_set_margin_top(button_box, 10);
    gtk_box_append(GTK_BOX(main_box), button_box);

    GtkWidget *clear_btn = gtk_button_new_with_label("Clear");
    gtk_box_append(GTK_BOX(button_box), clear_btn);
    g_signal_connect(clear_btn, "clicked", G_CALLBACK(on_clear_clicked), NULL);

    GtkWidget *backspace_btn = gtk_button_new_with_label("⌫");
    gtk_box_append(GTK_BOX(button_box), backspace_btn);
    g_signal_connect(backspace_btn, "clicked", G_CALLBACK(on_backspace_clicked), NULL);

    GtkWidget *history_btn = gtk_button_new_with_label("History");
    gtk_box_append(GTK_BOX(button_box), history_btn);
    g_signal_connect(history_btn, "clicked", G_CALLBACK(show_history), NULL);

    gtk_window_present(GTK_WINDOW(window));
}

int main(int argc, char **argv) {
    GtkApplication *app = gtk_application_new("com.example.calculator", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
    int status = g_application_run(G_APPLICATION(app), argc, argv);

    free_history();
    g_object_unref(app);
    return status;
}
