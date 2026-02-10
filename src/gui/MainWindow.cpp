#include "MainWindow.h"
#include "CalcEngine.h"
#include "HistoryModel.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QGroupBox>
#include <QToolBar>
#include <QClipboard>
#include <QApplication>
#include <QKeyEvent>
#include <QScrollBar>

// formatter.h already included via CalcEngine.h (with mpfr.h pre-included)

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_engine(new CalcEngine(this))
    , m_historyModel(new HistoryModel(this))
    , m_historyIndex(-1)
{
    setWindowTitle(QStringLiteral("High-Precision Calculator"));
    setMinimumSize(500, 600);
    resize(560, 700);

    setupUI();
    updatePrecisionLabel();
}

void MainWindow::setupUI()
{
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    mainLayout->setSpacing(6);

    setupToolbar();
    setupHistory();
    setupExpressionInput();

    mainLayout->addWidget(m_historyView, 1);

    // Expression row
    auto *exprLayout = new QHBoxLayout;
    exprLayout->addWidget(m_expressionInput, 1);
    auto *evalBtn = new QPushButton(QStringLiteral("="));
    evalBtn->setFixedWidth(50);
    evalBtn->setFixedHeight(36);
    QFont evalFont = evalBtn->font();
    evalFont.setPointSize(14);
    evalFont.setBold(true);
    evalBtn->setFont(evalFont);
    connect(evalBtn, &QPushButton::clicked, this, &MainWindow::onEvaluate);
    exprLayout->addWidget(evalBtn);
    mainLayout->addLayout(exprLayout);

    setupButtonGrid(mainLayout);

    setCentralWidget(central);
}

void MainWindow::setupToolbar()
{
    auto *toolbar = addToolBar(QStringLiteral("Settings"));
    toolbar->setMovable(false);

    toolbar->addWidget(new QLabel(QStringLiteral(" Precision: ")));

    m_precisionCombo = new QComboBox;
    m_precisionCombo->addItem(QStringLiteral("53 bits (double)"), 53);
    m_precisionCombo->addItem(QStringLiteral("128 bits"), 128);
    m_precisionCombo->addItem(QStringLiteral("256 bits"), 256);
    m_precisionCombo->addItem(QStringLiteral("512 bits"), 512);
    m_precisionCombo->addItem(QStringLiteral("1024 bits"), 1024);
    m_precisionCombo->addItem(QStringLiteral("4096 bits"), 4096);
    m_precisionCombo->addItem(QStringLiteral("8192 bits"), 8192);
    // Select 256 bits as default
    m_precisionCombo->setCurrentIndex(2);
    connect(m_precisionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPrecisionChanged);
    toolbar->addWidget(m_precisionCombo);

    m_precisionLabel = new QLabel;
    toolbar->addWidget(m_precisionLabel);

    toolbar->addSeparator();
    toolbar->addWidget(new QLabel(QStringLiteral(" Mode: ")));

    m_modeCombo = new QComboBox;
    m_modeCombo->addItem(QStringLiteral("Smart"), FORMAT_SMART);
    m_modeCombo->addItem(QStringLiteral("Fixed"), FORMAT_FIXED);
    m_modeCombo->addItem(QStringLiteral("Scientific"), FORMAT_SCIENTIFIC);
    m_modeCombo->addItem(QStringLiteral("Auto"), FORMAT_AUTO);
    connect(m_modeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onModeChanged);
    toolbar->addWidget(m_modeCombo);

    // Spacer
    auto *spacer = new QWidget;
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    toolbar->addWidget(spacer);

    // Clear history button
    auto *clearHistBtn = new QPushButton(QStringLiteral("Clear History"));
    connect(clearHistBtn, &QPushButton::clicked, this, &MainWindow::onClearHistory);
    toolbar->addWidget(clearHistBtn);
}

void MainWindow::setupHistory()
{
    m_historyView = new QListView;
    m_historyView->setModel(m_historyModel);
    m_historyView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_historyView->setSelectionMode(QAbstractItemView::SingleSelection);
    m_historyView->setWordWrap(true);

    QFont histFont(QStringLiteral("monospace"), 11);
    m_historyView->setFont(histFont);

    connect(m_historyView, &QListView::clicked, this, &MainWindow::onHistoryClicked);
}

void MainWindow::setupExpressionInput()
{
    m_expressionInput = new QLineEdit;
    m_expressionInput->setPlaceholderText(QStringLiteral("Enter expression (e.g., sqrt(2) + pi)"));
    QFont inputFont(QStringLiteral("monospace"), 13);
    m_expressionInput->setFont(inputFont);
    m_expressionInput->setFixedHeight(36);

    connect(m_expressionInput, &QLineEdit::returnPressed, this, &MainWindow::onEvaluate);

    // Install event filter for Up/Down arrow history navigation
    m_expressionInput->installEventFilter(this);
}

void MainWindow::setupButtonGrid(QVBoxLayout *mainLayout)
{
    auto *buttonGroup = new QGroupBox;
    auto *grid = new QGridLayout(buttonGroup);
    grid->setSpacing(3);

    // Button definitions: {label, text to insert}
    // Row 0: Scientific functions
    struct ButtonDef {
        const char *label;
        const char *insert;
    };

    ButtonDef row0[] = {
        {"sin",  "sin("},  {"cos",  "cos("},  {"tan",   "tan("},
        {"log",  "log("},  {"ln",   "log("},  {"exp",   "exp("},
        {"sqrt", "sqrt("}, {"^",    "^"}
    };
    ButtonDef row1[] = {
        {"asin",  "asin("}, {"acos",  "acos("}, {"atan",  "atan("},
        {"log10", "log10("}, {"e",    "e"},      {"pi",    "pi"},
        {"abs",   "abs("},  {"(",     "("}
    };
    ButtonDef row2[] = {
        {"7", "7"}, {"8", "8"}, {"9", "9"}, {"/", "/"},
        {")", ")"}, {"DEL", ""}, {"C", ""}, {"ans", ""}
    };
    ButtonDef row3[] = {
        {"4", "4"}, {"5", "5"}, {"6", "6"}, {"*", "*"},
        {"floor", "floor("}, {"ceil", "ceil("}
    };
    ButtonDef row4[] = {
        {"1", "1"}, {"2", "2"}, {"3", "3"}, {"-", "-"}
    };
    ButtonDef row5[] = {
        {"0", "0"}, {".", "."}, {",", ","}, {"+", "+"}
    };

    auto addRow = [&](ButtonDef *defs, int count, int row) {
        for (int col = 0; col < count; ++col) {
            auto *btn = new QPushButton(QString::fromUtf8(defs[col].label));
            btn->setMinimumHeight(38);
            btn->setMinimumWidth(48);

            QString label = QString::fromUtf8(defs[col].label);
            QString insert = QString::fromUtf8(defs[col].insert);

            if (label == QStringLiteral("DEL")) {
                connect(btn, &QPushButton::clicked, this, &MainWindow::onBackspace);
            } else if (label == QStringLiteral("C")) {
                connect(btn, &QPushButton::clicked, this, &MainWindow::onClearExpression);
            } else if (label == QStringLiteral("ans")) {
                connect(btn, &QPushButton::clicked, this, [this]() {
                    m_expressionInput->insert(m_historyModel->lastResult());
                    m_expressionInput->setFocus();
                });
            } else {
                connect(btn, &QPushButton::clicked, this, [this, insert]() {
                    onButtonClicked(insert);
                });
            }

            grid->addWidget(btn, row, col);
        }
    };

    addRow(row0, 8, 0);
    addRow(row1, 8, 1);
    addRow(row2, 8, 2);
    addRow(row3, 6, 3);
    addRow(row4, 4, 4);
    addRow(row5, 4, 5);

    mainLayout->addWidget(buttonGroup);
}

void MainWindow::onEvaluate()
{
    QString expr = m_expressionInput->text().trimmed();
    if (expr.isEmpty())
        return;

    // Add to input history
    m_inputHistory.append(expr);
    m_historyIndex = m_inputHistory.size();

    QString result = m_engine->evaluate(expr);

    if (m_engine->hasError()) {
        m_historyModel->addEntry(expr, m_engine->lastError(), true);
    } else {
        m_historyModel->addEntry(expr, result, false);
    }

    m_expressionInput->clear();

    // Scroll to bottom
    m_historyView->scrollToBottom();
}

void MainWindow::onButtonClicked(const QString &text)
{
    m_expressionInput->insert(text);
    m_expressionInput->setFocus();
}

void MainWindow::onPrecisionChanged(int index)
{
    int bits = m_precisionCombo->itemData(index).toInt();
    m_engine->setPrecision(bits);
    updatePrecisionLabel();
}

void MainWindow::onModeChanged(int index)
{
    auto format = static_cast<NumberFormat>(m_modeCombo->itemData(index).toInt());
    m_engine->setFormat(format);
}

void MainWindow::onHistoryClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QString result = m_historyModel->data(index, HistoryModel::ResultRole).toString();
    bool isError = m_historyModel->data(index, HistoryModel::IsErrorRole).toBool();

    if (!isError) {
        QApplication::clipboard()->setText(result);
    }
}

void MainWindow::onClearExpression()
{
    m_expressionInput->clear();
    m_expressionInput->setFocus();
}

void MainWindow::onBackspace()
{
    m_expressionInput->backspace();
    m_expressionInput->setFocus();
}

void MainWindow::onClearHistory()
{
    m_historyModel->clear();
}

void MainWindow::updatePrecisionLabel()
{
    m_precisionLabel->setText(
        QStringLiteral(" (~%1 digits)").arg(m_engine->decimalDigits()));
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_expressionInput && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);

        if (keyEvent->key() == Qt::Key_Up) {
            if (!m_inputHistory.isEmpty() && m_historyIndex > 0) {
                --m_historyIndex;
                m_expressionInput->setText(m_inputHistory.at(m_historyIndex));
            }
            return true;
        }

        if (keyEvent->key() == Qt::Key_Down) {
            if (m_historyIndex < m_inputHistory.size() - 1) {
                ++m_historyIndex;
                m_expressionInput->setText(m_inputHistory.at(m_historyIndex));
            } else {
                m_historyIndex = m_inputHistory.size();
                m_expressionInput->clear();
            }
            return true;
        }
    }

    return QMainWindow::eventFilter(obj, event);
}
