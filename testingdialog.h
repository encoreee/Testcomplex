#ifndef TESTINGDIALOG_H
#define TESTINGDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE

namespace Ui {
class TestingDialog;
}

class QIntValidator;

QT_END_NAMESPACE

class TestingDialog : public QDialog
{
    Q_OBJECT

signals:

void settingsChanged();

public:

    enum class Tests
    {
               SNR,
               WarmingUp,
               Crush
    };

    struct TestSettings
    {
        Tests                     choosenTest;
        unsigned int              pollingFrequency;
        int                       paramert3;
        int                       paramert4;
    };

    explicit TestingDialog(QWidget *parent = nullptr);
    ~TestingDialog();

    TestSettings testsettings() const;
    TestSettings createTest();
    bool create();
    bool cancel();

private:
    void fillParameters();
    void updateSettings();


private:
    Ui::TestingDialog *m_testui = nullptr;
    TestSettings m_currentTestSettings;
    QIntValidator *m_intTestValidator = nullptr;



};
#endif // TESTINGDIALOG_H
