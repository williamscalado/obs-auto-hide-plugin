#include "settings-dialog.hpp"
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QNetworkReply>
#include <QScrollArea>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QComboBox>
#include <QListWidget>
#include <QCheckBox>
#include <QTabWidget>
#include <obs-module.h>

SettingsDialog::SettingsDialog(PluginConfig &config,
                               SceneController *controller, QWidget *parent)
    : QDialog(parent), config(config), scene_controller(controller) {
    network_manager = new QNetworkAccessManager(this);
    setWindowTitle("Cidade Viva - Configurações");
    setMinimumSize(650, 550);

    setup_ui();
    load_current_values();
}

SettingsDialog::~SettingsDialog() {}

void SettingsDialog::setup_ui() {
    QString style_sheet = R"(
        QDialog {
            background-color: #2b2b2b;
            color: #ffffff;
            font-family: 'Segoe UI', sans-serif;
            font-size: 14px;
        }
        QTabWidget::pane {
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            background-color: #323438;
            top: -1px;
        }
        QTabBar::tab {
            background-color: #2b2b2b;
            color: #cccccc;
            padding: 12px 24px;
            margin-right: 2px;
            border: 1px solid #3d3d3d;
            border-bottom: none;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background-color: #323438;
            color: #ffffff;
            font-weight: bold;
            border-bottom: 2px solid #007acc;
        }
        QTabBar::tab:hover:!selected {
            background-color: #3d3d3d;
        }
        QGroupBox {
            background-color: transparent;
            border: 1px solid #3d3d3d;
            border-radius: 6px;
            margin-top: 12px;
            padding: 20px 20px 15px 20px;
            font-weight: bold;
            font-size: 13px;
            color: #e0e0e0;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            padding: 4px 10px;
            left: 8px;
            top: 0px;
            background-color: transparent;
        }
        QLabel {
            color: #cccccc;
            padding: 2px 0px;
        }
        QLineEdit, QSpinBox, QComboBox {
            background-color: #1e1e1e;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            padding: 8px 10px;
            color: #ffffff;
            selection-background-color: #007acc;
            min-height: 28px;
        }
        QLineEdit:focus, QSpinBox:focus, QComboBox:focus {
            border: 1px solid #007acc;
        }
        QComboBox::drop-down {
            subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 25px;
            border-left: 1px solid #3d3d3d;
        }
        QComboBox::down-arrow {
            image: none;
            border-left: 4px solid transparent;
            border-right: 4px solid transparent;
            border-top: 6px solid #cccccc;
            margin-right: 5px;
        }
        QPushButton {
            background-color: #3d3d3d;
            border: 1px solid #505050;
            border-radius: 4px;
            padding: 10px 20px;
            color: white;
            font-weight: bold;
            min-height: 32px;
        }
        QPushButton:hover {
            background-color: #4d4d4d;
        }
        QPushButton:pressed {
            background-color: #2d2d2d;
        }
        QPushButton[default="true"] {
            background-color: #007acc;
            border: 1px solid #005c99;
        }
        QPushButton[default="true"]:hover {
            background-color: #008ae6;
        }
        QListWidget {
            background-color: #1e1e1e;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            padding: 5px;
        }
        QListWidget::item {
            padding: 10px;
            border-bottom: 1px solid #2d2d2d;
        }
        QCheckBox {
            spacing: 10px;
            padding: 5px 0px;
        }
        QCheckBox::indicator {
            width: 20px;
            height: 20px;
        }
    )";
    setStyleSheet(style_sheet);

    QVBoxLayout *main_layout = new QVBoxLayout(this);
    main_layout->setSpacing(15);
    main_layout->setContentsMargins(15, 15, 15, 15);

    // Criar TabWidget
    QTabWidget *tab_widget = new QTabWidget(this);

    // === ABA 1: CONEXÃO ===
    QWidget *tab_connection = new QWidget();
    QVBoxLayout *layout_connection = new QVBoxLayout(tab_connection);
    layout_connection->setContentsMargins(20, 20, 20, 20);
    layout_connection->setSpacing(15);

    QGroupBox *group_holyrics = new QGroupBox("Configurações de Conexão", tab_connection);
    QVBoxLayout *layout_holyrics = new QVBoxLayout(group_holyrics);
    layout_holyrics->setSpacing(12);
    layout_holyrics->setContentsMargins(5, 5, 5, 8);

    QFormLayout *form_holyrics = new QFormLayout();
    form_holyrics->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form_holyrics->setFormAlignment(Qt::AlignLeft | Qt::AlignTop);
    form_holyrics->setVerticalSpacing(12);
    form_holyrics->setHorizontalSpacing(15);
    form_holyrics->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    url_input = new QLineEdit(tab_connection);
    url_input->setPlaceholderText("http://localhost:9000");
    url_input->setMinimumWidth(300);
    form_holyrics->addRow("URL Base:", url_input);

    interval_input = new QSpinBox(tab_connection);
    interval_input->setRange(100, 10000);
    interval_input->setSuffix(" ms");
    interval_input->setSingleStep(100);
    interval_input->setMinimumWidth(150);
    form_holyrics->addRow("Intervalo (Polling):", interval_input);

    layout_holyrics->addLayout(form_holyrics);

    QHBoxLayout *test_layout = new QHBoxLayout();
    test_layout->setSpacing(15);

    test_button = new QPushButton("Testar Conexão", tab_connection);
    test_button->setCursor(Qt::PointingHandCursor);
    test_button->setMaximumWidth(180);

    status_label = new QLabel("Status: ● Aguardando teste", tab_connection);
    status_label->setStyleSheet("font-weight: bold; color: #888; padding-left: 5px;");

    test_layout->addWidget(test_button);
    test_layout->addWidget(status_label, 1);
    layout_holyrics->addLayout(test_layout);

    connect(test_button, &QPushButton::clicked, this, &SettingsDialog::test_connection);

    layout_connection->addWidget(group_holyrics);
    layout_connection->addStretch();

    tab_widget->addTab(tab_connection, "🔌 Conexão");

    // === ABA 2: CENAS ===
    QWidget *tab_scenes = new QWidget();
    QVBoxLayout *layout_scenes_tab = new QVBoxLayout(tab_scenes);
    layout_scenes_tab->setContentsMargins(20, 20, 20, 20);
    layout_scenes_tab->setSpacing(15);

    QGroupBox *group_scenes = new QGroupBox("Controle de Cenas", tab_scenes);
    QVBoxLayout *layout_scenes = new QVBoxLayout(group_scenes);
    layout_scenes->setSpacing(12);
    layout_scenes->setContentsMargins(5, 5, 5, 8);

    QFormLayout *form_scenes = new QFormLayout();
    form_scenes->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form_scenes->setVerticalSpacing(12);
    form_scenes->setHorizontalSpacing(15);
    form_scenes->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    scene_combo = new QComboBox(tab_scenes);
    scene_combo->addItems(scene_controller->get_available_scenes());
    scene_combo->setCursor(Qt::PointingHandCursor);
    scene_combo->setMinimumWidth(300);
    form_scenes->addRow("Cena para Monitorar:", scene_combo);

    layout_scenes->addLayout(form_scenes);

    connect(scene_combo, &QComboBox::currentTextChanged, this, &SettingsDialog::on_scene_changed);

    layout_scenes->addSpacing(5);
    QLabel *lbl_sources = new QLabel("Fontes para ocultar (Check para esconder):", tab_scenes);
    lbl_sources->setStyleSheet("font-weight: normal; padding: 3px 0px;");
    layout_scenes->addWidget(lbl_sources);

    sources_list = new QListWidget(tab_scenes);
    sources_list->setSelectionMode(QAbstractItemView::NoSelection);
    sources_list->setAlternatingRowColors(true);
    sources_list->setStyleSheet("QListWidget { alternate-background-color: #252525; }");
    sources_list->setMinimumHeight(180);
    layout_scenes->addWidget(sources_list);

    layout_scenes_tab->addWidget(group_scenes);

    tab_widget->addTab(tab_scenes, "🎬 Cenas");

    // === ABA 3: COMPORTAMENTO ===
    QWidget *tab_behavior = new QWidget();
    QVBoxLayout *layout_behavior_tab = new QVBoxLayout(tab_behavior);
    layout_behavior_tab->setContentsMargins(20, 20, 20, 20);
    layout_behavior_tab->setSpacing(15);

    QGroupBox *group_behavior = new QGroupBox("Automação e Preferências", tab_behavior);
    QVBoxLayout *layout_behavior = new QVBoxLayout(group_behavior);
    layout_behavior->setSpacing(8);
    layout_behavior->setContentsMargins(5, 5, 5, 8);

    restore_state_check = new QCheckBox("Restaurar estado anterior das fontes", tab_behavior);
    restore_state_check->setToolTip("Ao fim do versículo, as fontes voltarão a ficar visíveis se estavam antes.");
    layout_behavior->addWidget(restore_state_check);

    QFormLayout *form_behavior = new QFormLayout();
    form_behavior->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form_behavior->setVerticalSpacing(12);
    form_behavior->setHorizontalSpacing(15);
    form_behavior->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    delay_input = new QSpinBox(tab_behavior);
    delay_input->setRange(0, 5000);
    delay_input->setSuffix(" ms");
    delay_input->setMinimumWidth(150);
    form_behavior->addRow("Delay de ação:", delay_input);

    layout_behavior->addLayout(form_behavior);
    layout_behavior->addSpacing(5);

    notifications_check = new QCheckBox("Mostrar notificações do sistema", tab_behavior);
    layout_behavior->addWidget(notifications_check);

    auto_activate_check = new QCheckBox("Ativar monitoramento ao iniciar o OBS", tab_behavior);
    layout_behavior->addWidget(auto_activate_check);

    disable_in_music_check = new QCheckBox("Pausar monitoramento se for música", tab_behavior);
    layout_behavior->addWidget(disable_in_music_check);

    layout_behavior_tab->addWidget(group_behavior);
    layout_behavior_tab->addStretch();

    tab_widget->addTab(tab_behavior, "⚙️ Comportamento");

    main_layout->addWidget(tab_widget);

    // Botões finais
    QHBoxLayout *buttons = new QHBoxLayout();
    buttons->setContentsMargins(0, 5, 0, 0);
    buttons->setSpacing(10);

    QPushButton *btn_cancel = new QPushButton("Cancelar", this);
    btn_cancel->setCursor(Qt::PointingHandCursor);
    btn_cancel->setMinimumWidth(120);

    QPushButton *btn_save = new QPushButton("Salvar Alterações", this);
    btn_save->setDefault(true);
    btn_save->setCursor(Qt::PointingHandCursor);
    btn_save->setMinimumWidth(180);

    connect(btn_save, &QPushButton::clicked, this, &SettingsDialog::save);
    connect(btn_cancel, &QPushButton::clicked, this, &QDialog::reject);

    buttons->addStretch();
    buttons->addWidget(btn_cancel);
    buttons->addWidget(btn_save);

    main_layout->addLayout(buttons);
}

void SettingsDialog::load_current_values() {
    url_input->setText(config.holyrics_url);
    interval_input->setValue(config.polling_interval_ms);

    scene_combo->setCurrentText(config.monitored_scene);
    on_scene_changed(config.monitored_scene);

    restore_state_check->setChecked(config.restore_previous_state);
    delay_input->setValue(config.action_delay_ms);
    notifications_check->setChecked(config.show_notifications);
    auto_activate_check->setChecked(config.auto_activate);
    disable_in_music_check->setChecked(config.disable_in_music);
}

void SettingsDialog::on_scene_changed(const QString &scene_name) {
    sources_list->clear();
    if (scene_name.isEmpty()) return;

    QStringList current_sources = scene_controller->get_scene_sources(scene_name);

    for (const QString &source : current_sources) {
        QListWidgetItem *item = new QListWidgetItem(source);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);

        if (config.sources_to_hide.contains(source)) {
            item->setCheckState(Qt::Checked);
        } else {
            item->setCheckState(Qt::Unchecked);
        }
        sources_list->addItem(item);
    }
}

void SettingsDialog::test_connection() {
    QString url = url_input->text();
    if (url.endsWith("/")) url.chop(1);
    url += "/view/text";

    test_button->setEnabled(false);
    test_button->setText("Testando...");
    status_label->setText("⏳ Conectando...");
    status_label->setStyleSheet("font-weight: bold; color: #ffcc00;");

    QNetworkRequest request(url);
    request.setTransferTimeout(2000);

    QNetworkReply *reply = network_manager->get(request);

    connect(reply, &QNetworkReply::finished, [this, reply]() {
        test_button->setEnabled(true);
        test_button->setText("Testar Conexão");

        if (reply->error() != QNetworkReply::NoError) {
            status_label->setText("❌ Falha na conexão");
            status_label->setStyleSheet("font-weight: bold; color: #ff5555;");
            QMessageBox::warning(this, "Erro", "Não foi possível conectar ao Holyrics.\n" + reply->errorString());
        } else {
            QString html = reply->readAll();
            status_label->setText("✅ Conectado com sucesso");
            status_label->setStyleSheet("font-weight: bold; color: #55ff55;");

            bool has_verse = html.contains("class=\"bible_slide") || html.contains("<desc>");
            QString msg = has_verse ? "📖 Versículo detectado." : "✓ Nenhum versículo no momento.";
            QMessageBox::information(this, "Sucesso", "Conexão OK!\n" + msg);
        }
        reply->deleteLater();
    });
}

void SettingsDialog::save() {
    config.holyrics_url = url_input->text();
    config.polling_interval_ms = interval_input->value();
    config.monitored_scene = scene_combo->currentText();

    config.sources_to_hide.clear();
    for (int i = 0; i < sources_list->count(); ++i) {
        QListWidgetItem *item = sources_list->item(i);
        if (item->checkState() == Qt::Checked) {
            config.sources_to_hide.append(item->text());
        }
    }

    config.restore_previous_state = restore_state_check->isChecked();
    config.action_delay_ms = delay_input->value();
    config.show_notifications = notifications_check->isChecked();
    config.auto_activate = auto_activate_check->isChecked();
    config.disable_in_music = disable_in_music_check->isChecked();

    accept();
}

void SettingsDialog::add_source_manually() {}
