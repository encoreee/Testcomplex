#ifndef TESTINGDIALOG_H
#define TESTINGDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE

namespace Ui {
class TestingDialog;
}

class QIntValidator;

QT_END_NAMESPACE

enum class Tests
{
           SNR,
           WarmingUp,
           Crush
};

struct TestSettings
{
    Tests                     choosenTest;
    int                       pollingFrequency;
    int                       numberOfCycles;
    int                       paramert4;
};

class TestingDialog : public QDialog
{
    Q_OBJECT

signals:

void settingsChanged();

public:

    explicit TestingDialog(QWidget *parent = nullptr);
    ~TestingDialog();

    TestSettings testsettings() const;
    TestSettings createTest();
    bool create();
    bool cancel();

private:

    void fillParameters();
    void updateSettings();
    void checkCustomPollingFrequency(int idx);
    void checkCustomCustomCyclesNumber(int idx);

    Ui::TestingDialog *m_testui = nullptr;
    TestSettings m_currentTestSettings;
    QIntValidator *m_intFrequencyValidator = nullptr;
    QIntValidator *m_intCyclesNumberVaildator = nullptr;



};
#endif // TESTINGDIALOG_H
