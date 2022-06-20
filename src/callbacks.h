#include <gtk/gtk.h>


void
on_winMain_show                        (GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_winMain_delete_event                (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_mnuMessengerConnect_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mnuDisconnect_activate              (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mnuPreferences_activate             (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mnuMessengerQuit_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_mnuHelpAbout_activate               (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_butSendIM_clicked                   (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_tbAddBuddy_clicked                  (GtkToolButton   *toolbutton,
                                        gpointer         user_data);

void
on_tvBuddyList_row_activated           (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_cboStatus_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data);

gboolean
on_winIM_delete_event                  (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_mnuConversationSave_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_quit1_activate                      (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_butIMSendFile_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_butImInvite_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butSmiley_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butIMFont_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butIMBuzz_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butIMAddBuddy_clicked               (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_txtIMSend_key_press_event           (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
on_butIMSend_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_mnuConversationQuit_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_butEAlien_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEAngel_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEAngry_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEBatting_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEBeatUp_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEBigSmile_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEBlush_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEBrokenHeart_clicked             (GtkButton       *button,
                                        gpointer         user_data);

void
on_butBye_clicked                      (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEClap_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEClown_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butCowboy_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butECry_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEDance_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEDevil_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEDoh_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEEyebrow_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEEyeRoll_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEFrustrated_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEDead_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEGiggle_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEGlasses_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEPbbth_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEPray_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butERofl_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butESad_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEShamrock_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEShh_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_butESick_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butESilent_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_butESweat_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEShame_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEHugs_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEHypnotized_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEIdea_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEKiss_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butELaugh_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butELiar_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butELoser_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butELove_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEMean_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEMoneyEyes_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEStraightFace_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEHaw_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEPeace_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butENailBiting_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEMaleFighter2_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEQuestion_clicked                (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEDrool_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEMonkey_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEChicken_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEPumpkin_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEFlower_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_butESigh_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butESilly_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butECow_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEPig_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEFemaleFighter_clicked           (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEMaleFighter1_clicked            (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEParty_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butECool_clicked                    (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEThink_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butETired_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butETongue_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEYinYang_clicked                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEYouKidding_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEAlien2_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_winSmiley_delete_event              (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_butEUSFlag_activate                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butECoffee_activate                 (GtkButton       *button,
                                        gpointer         user_data);

void
on_butESleep_activate                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_butESmile_activate                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEStar_activate                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEWaiting_activate                (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEWave_activate                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEWhistle_activate                (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEWink_activate                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEWorried_activate                (GtkButton       *button,
                                        gpointer         user_data);

void
on_butEWorship_activate                (GtkButton       *button,
                                        gpointer         user_data);

void
on_butETalk2Hand_activate              (GtkButton       *button,
                                        gpointer         user_data);

void
on_mnuMessengerRefresh_activate        (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
