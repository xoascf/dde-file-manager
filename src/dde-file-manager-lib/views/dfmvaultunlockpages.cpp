/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     luzhen<luzhen@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             luzhen<luzhen@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define TOOLTIP_SHOW_DURATION 3000

#include "dfmvaultunlockpages.h"
#include "vault/interfaceactivevault.h"
#include "controllers/vaultcontroller.h"
#include "dfilemanagerwindow.h"
#include "accessibility/ac-lib-file-manager.h"

#include <DPushButton>
#include <DPasswordEdit>
#include <DFloatingWidget>
#include <DToolTip>

#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QToolTip>
#include <QEvent>
#include <QTimer>

DFMVaultUnlockPages::DFMVaultUnlockPages(QWidget *parent)
    : DFMVaultPageBase(parent)
{
    pTooltipTimer = new QTimer(this);
    connect(pTooltipTimer, &QTimer::timeout,
            this, &DFMVaultUnlockPages::slotTooltipTimerTimeout);

    AC_SET_ACCESSIBLE_NAME(this, AC_VAULT_PASSWORD_UNLOCK_WIDGET);

    setIcon(QIcon::fromTheme("dfm_vault"));
    setFixedWidth(396);

    // 标题
    QLabel *pTitle = new QLabel(tr("Unlock File Vault"), this);
    AC_SET_ACCESSIBLE_NAME(pTitle, AC_VAULT_PASSWORD_UNLOCK_TITLE);
    QFont font = pTitle->font();
    font.setPixelSize(18);
    pTitle->setFont(font);
    pTitle->setAlignment(Qt::AlignHCenter);

    // 信息
    QLabel *pMessage = new QLabel(tr("Verify your password"), this);
    AC_SET_ACCESSIBLE_NAME(pMessage, AC_VAULT_PASSWORD_UNLOCK_CONTENT);
    pMessage->setAlignment(Qt::AlignHCenter);

    // 密码编辑框
    m_passwordEdit = new DPasswordEdit(this);
    AC_SET_ACCESSIBLE_NAME(m_passwordEdit, AC_VAULT_PASSWORD_UNLOCK_EDIT);
    m_passwordEdit->lineEdit()->setPlaceholderText(tr("Password"));
    m_passwordEdit->lineEdit()->installEventFilter(this);
    m_passwordEdit->lineEdit()->setAttribute(Qt::WA_InputMethodEnabled, false);

    // 提示按钮
    m_tipsButton = new QPushButton(this);
    AC_SET_ACCESSIBLE_NAME(m_tipsButton, AC_VAULT_PASSWORD_UNLOCK_HINT_BUTTON);
    m_tipsButton->setIcon(QIcon(":/icons/images/icons/light_32px.svg"));

    // 主视图
    QFrame *mainFrame = new QFrame(this);

    // 布局
    QHBoxLayout *play1 = new QHBoxLayout();
    play1->setMargin(0);
    play1->addWidget(m_passwordEdit);
    play1->addWidget(m_tipsButton);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainFrame);
    mainLayout->setMargin(0);
    mainLayout->addWidget(pTitle);
    mainLayout->addWidget(pMessage);
    mainLayout->addLayout(play1);

    mainFrame->setLayout(mainLayout);
    addContent(mainFrame);
    setSpacing(0);
    // 防止点击按钮后界面隐藏
    setOnButtonClickedClose(false);

    QStringList btnList({tr("Cancel","button"), tr("Unlock","button")});
    addButton(btnList[0], false);
    addButton(btnList[1], true, ButtonType::ButtonRecommend);
    getButton(1)->setEnabled(false);

    AC_SET_ACCESSIBLE_NAME(getButton(0), AC_VAULT_PASSWORD_UNLOCK_CANCEL_BUTTON);
    AC_SET_ACCESSIBLE_NAME(getButton(1), AC_VAULT_PASSWORD_UNLOCK_OK_BUTTON);

    connect(this, &DFMVaultUnlockPages::buttonClicked, this, &DFMVaultUnlockPages::onButtonClicked);
    connect(m_passwordEdit, &DPasswordEdit::textChanged, this, &DFMVaultUnlockPages::onPasswordChanged);
    connect(VaultController::ins(), &VaultController::signalUnlockVault, this, &DFMVaultUnlockPages::onVaultUlocked);
    connect(VaultController::ins(), &VaultController::sigRestorePasswordInput, this, &DFMVaultUnlockPages::restorePasswordInput);
    connect(m_tipsButton, &QPushButton::clicked, this, [this] {
        QString strPwdHint("");
        if (InterfaceActiveVault::getPasswordHint(strPwdHint))
        {
            QString hint = tr("Password hint: %1").arg(strPwdHint);
            showToolTip(hint, TOOLTIP_SHOW_DURATION, EN_ToolTip::Information);
        }
    });
    connect(this, &DFMVaultPageBase::accepted, this, &DFMVaultPageBase::enterVaultDir);
}

void DFMVaultUnlockPages::showEvent(QShowEvent *event)
{
    // 重置所有控件状态
    m_passwordEdit->lineEdit()->clear();
    QLineEdit edit;
    QPalette palette = edit.palette();
    m_passwordEdit->lineEdit()->setPalette(palette);
    m_passwordEdit->setEchoMode(QLineEdit::Password);
    m_bUnlockByPwd = false;

    // 如果密码提示信息为空，则隐藏提示按钮
    QString strPwdHint("");
    if (InterfaceActiveVault::getPasswordHint(strPwdHint)) {
        if (strPwdHint.isEmpty()) {
            m_tipsButton->hide();
        } else {
            m_tipsButton->show();
        }
    }
    event->accept();
}

void DFMVaultUnlockPages::showToolTip(const QString &text, int duration, DFMVaultUnlockPages::EN_ToolTip enType)
{
    if (!m_tooltip) {
        m_tooltip = new DToolTip(text);
        m_tooltip->setObjectName("AlertTooltip");
        m_tooltip->setWordWrap(true);

        m_frame = new DFloatingWidget;
        m_frame->setFramRadius(DStyle::pixelMetric(style(), DStyle::PM_FrameRadius));
        m_frame->setStyleSheet("background-color: rgba(247, 247, 247, 0.6);");
        m_frame->setWidget(m_tooltip);
    }
    if (EN_ToolTip::Warning == enType)
        m_tooltip->setForegroundRole(DPalette::TextWarning);
    else
        m_tooltip->setForegroundRole(DPalette::TextTitle);

    m_frame->setParent(this);

    m_tooltip->setText(text);
    if (m_frame->parent()) {
        m_frame->setGeometry(8, 154, 68, 26);
        m_frame->show();
        m_frame->adjustSize();
        m_frame->raise();
    }

    if (duration < 0) {
        return;
    }

    // 重新启动定时器，定时隐藏tooltip
    if (pTooltipTimer) {
        if (pTooltipTimer->isActive())
            pTooltipTimer->stop();
        pTooltipTimer->start(duration);
    }
}

DFMVaultUnlockPages *DFMVaultUnlockPages::instance()
{
    static DFMVaultUnlockPages s_instance;
    return &s_instance;
}

void DFMVaultUnlockPages::hideEvent(QHideEvent *event)
{
    if (m_frame)
        m_frame->close();
    DFMVaultPageBase::hideEvent(event);
}

void DFMVaultUnlockPages::onButtonClicked(const int &index)
{
    if (index == 1) {
        // 点击解锁后,灰化解锁按钮
        getButton(1)->setEnabled(false);

        // 判断保险箱剩余错误输入次数
        VaultController *pVaultController = VaultController::ins();
        int nLeftoverErrorTimes = pVaultController->getLeftoverErrorInputTimes();
        if (nLeftoverErrorTimes < 1) {
            int nNeedWaitMinutes = pVaultController->getNeedWaitMinutes();
            showToolTip(tr("Please try again %1 minutes later").arg(nNeedWaitMinutes), TOOLTIP_SHOW_DURATION, EN_ToolTip::Warning);
            return;
        }

        QString strPwd = m_passwordEdit->text();

        QString strCipher("");
        if (InterfaceActiveVault::checkPassword(strPwd, strCipher)) {
            m_bUnlockByPwd = true;
            pVaultController->unlockVault(strCipher);
            // 密码输入正确后，剩余输入次数还原,需要等待的分钟数还原
            pVaultController->restoreLeftoverErrorInputTimes();
            pVaultController->restoreNeedWaitMinutes();
        } else {
            // 设置密码输入框颜色
            // 修复bug-51508 激活密码框警告状态
            m_passwordEdit->setAlert(true);


            // 保险箱剩余错误密码输入次数减1
            pVaultController->leftoverErrorInputTimesMinusOne();
            // 显示错误输入提示
            nLeftoverErrorTimes = pVaultController->getLeftoverErrorInputTimes();
            if (nLeftoverErrorTimes < 1) {
                // 计时10分钟后，恢复密码编辑框
                pVaultController->startTimerOfRestorePasswordInput();
                // 错误输入次数超过了限制
                int nNeedWaitMinutes = pVaultController->getNeedWaitMinutes();
                showToolTip(tr("Wrong password, please try again %1 minutes later").arg(nNeedWaitMinutes), TOOLTIP_SHOW_DURATION, EN_ToolTip::Warning);
            } else {
                if (nLeftoverErrorTimes == 1)
                    showToolTip(tr("Wrong password, one chance left"), TOOLTIP_SHOW_DURATION, EN_ToolTip::Warning);
                else
                    showToolTip(tr("Wrong password, %1 chances left").arg(nLeftoverErrorTimes), TOOLTIP_SHOW_DURATION, EN_ToolTip::Warning);
            }
        }
        return;
    }

    close();
}

void DFMVaultUnlockPages::onPasswordChanged(const QString &pwd)
{
    if (!pwd.isEmpty()) {
        getButton(1)->setEnabled(true);
        // 修复bug-51508 取消密码框警告状态
        m_passwordEdit->setAlert(false);
    } else {
        getButton(1)->setEnabled(false);
    }
}

void DFMVaultUnlockPages::onVaultUlocked(int state)
{
    if (m_bUnlockByPwd) {
        if (state == 0) {
            emit accepted();
            close();
        } else if (state == 1) { // cryfs没有成功卸载挂载目录
            // 解决sp3bug-38885:注销系统时，cryfs卸载挂载目录会概率性失败
            // 卸载挂载目录
            QProcess process;
            QString fusermountBinary = QStandardPaths::findExecutable("fusermount");
            process.start(fusermountBinary, {"-zu", QString(VAULT_BASE_PATH) + QDir::separator() + QString(VAULT_DECRYPT_DIR_NAME)});
            process.waitForStarted();
            process.waitForFinished();
            process.terminate();
            if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0) {
                QString strPwd = m_passwordEdit->text();
                QString strCipher("");
                // 判断密码是否正确
                if (InterfaceActiveVault::checkPassword(strPwd, strCipher)) {
                    VaultController::ins()->unlockVault(strCipher);
                    return;
                } else {    // 密码不正确
                    // 设置密码输入框颜色,并弹出tooltip
                    m_passwordEdit->lineEdit()->setStyleSheet("background-color:rgba(241, 57, 50, 0.15)");
                    showToolTip(tr("Wrong password"), TOOLTIP_SHOW_DURATION, EN_ToolTip::Warning);
                }
            }
        } else {
            // error tips
            QString errMsg = tr("Failed to unlock file vault");
            DDialog dialog(this);
            dialog.setIcon(QIcon::fromTheme("dialog-warning"));
            dialog.setTitle(errMsg);
            dialog.addButton(tr("OK","button"), true, DDialog::ButtonRecommend);
            dialog.exec();
        }

        m_bUnlockByPwd = false;
    }
}

void DFMVaultUnlockPages::restorePasswordInput()
{
    // 密码剩余输入次数还原，需要等待的分钟数还原
    VaultController::ins()->restoreLeftoverErrorInputTimes();
    VaultController::ins()->restoreNeedWaitMinutes();
    // 设置密码框可以输入
    m_passwordEdit->lineEdit()->setEnabled(true);
}

void DFMVaultUnlockPages::slotTooltipTimerTimeout()
{
    m_frame->close();
}

