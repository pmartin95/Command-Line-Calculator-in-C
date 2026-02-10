#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLineEdit>
#include <QListView>
#include <QComboBox>
#include <QLabel>
#include <QStringList>
#include <QVBoxLayout>

class CalcEngine;
class HistoryModel;

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() = default;

private slots:
    void onEvaluate();
    void onButtonClicked(const QString &text);
    void onPrecisionChanged(int index);
    void onModeChanged(int index);
    void onEvalModeChanged(int index);
    void onHistoryClicked(const QModelIndex &index);
    void onClearExpression();
    void onBackspace();
    void onClearHistory();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;

private:
    void setupUI();
    void setupToolbar();
    void setupHistory();
    void setupExpressionInput();
    void setupButtonGrid(QVBoxLayout *mainLayout);
    void updatePrecisionLabel();

    CalcEngine *m_engine;
    HistoryModel *m_historyModel;

    QLineEdit *m_expressionInput;
    QListView *m_historyView;
    QComboBox *m_precisionCombo;
    QComboBox *m_modeCombo;
    QComboBox *m_evalModeCombo;
    QLabel *m_precisionLabel;

    // Input history navigation
    QStringList m_inputHistory;
    int m_historyIndex;
};

#endif // MAINWINDOW_H
