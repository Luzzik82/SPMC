/*
 *      Copyright (C) 2005-2015 Team XBMC
 *      http://xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, see
 *  <http://www.gnu.org/licenses/>.
 *
 */

#include "PVRActionListener.h"

#include "Application.h"
#include "ApplicationMessenger.h"
#include "guilib/Key.h"
#include "guilib/GUIWindow.h"
#include "guilib/LocalizeStrings.h"
#include "guilib/GUIWindowManager.h"
#include "dialogs/GUIDialogNumeric.h"
#include "settings/AdvancedSettings.h"
#include "settings/Settings.h"
#include "utils/log.h"
#include "utils/StringUtils.h"

#include "pvr/PVRManager.h"
#include "pvr/channels/PVRChannelGroupsContainer.h"

using namespace PVR;

CPVRActionListener::CPVRActionListener()
{
}

CPVRActionListener &CPVRActionListener::Get()
{
  static CPVRActionListener instance;
  return instance;
}

bool CPVRActionListener::OnAction(const CAction &action)
{
  switch (action.GetID())
  {
    case REMOTE_0:
    case REMOTE_1:
    case REMOTE_2:
    case REMOTE_3:
    case REMOTE_4:
    case REMOTE_5:
    case REMOTE_6:
    case REMOTE_7:
    case REMOTE_8:
    case REMOTE_9:
    {
      if (g_application.IsFullScreen() && g_application.CurrentFileItem().IsLiveTV())
      {
        if(g_PVRManager.IsPlaying())
        {
          // pvr client addon
          CPVRChannelPtr playingChannel;
          if(!g_PVRManager.GetCurrentChannel(playingChannel))
            return false;
          
          if (action.GetID() == REMOTE_0)
          {
            CPVRChannelGroupPtr group = g_PVRChannelGroups->GetPreviousPlayedGroup();
            if (group)
            {
              g_PVRManager.SetPlayingGroup(group);
              CFileItemPtr fileItem = group->GetLastPlayedChannel(playingChannel->ChannelID());
              if (fileItem && fileItem->HasPVRChannelInfoTag())
              {
                CLog::Log(LOGDEBUG, "%s - switch to channel number %d", __FUNCTION__, fileItem->GetPVRChannelInfoTag()->ChannelNumber());
                CApplicationMessenger::Get().SendAction(CAction(ACTION_CHANNEL_SWITCH, (float) fileItem->GetPVRChannelInfoTag()->ChannelNumber()), WINDOW_INVALID, false);
              }
            }
          }
          else
          {
            int autoCloseTime = CSettings::Get().GetBool("pvrplayback.confirmchannelswitch") ? 0 : g_advancedSettings.m_iPVRNumericChannelSwitchTimeout;
            std::string strChannel = StringUtils::Format("%i", action.GetID() - REMOTE_0);
            if (CGUIDialogNumeric::ShowAndGetNumber(strChannel, g_localizeStrings.Get(19000), autoCloseTime) || autoCloseTime)
            {
              int iChannelNumber = atoi(strChannel.c_str());
              if (iChannelNumber > 0 && iChannelNumber != playingChannel->ChannelNumber())
              {
                CPVRChannelGroupPtr selectedGroup = g_PVRManager.GetPlayingGroup(playingChannel->IsRadio());
                CFileItemPtr channel = selectedGroup->GetByChannelNumber(iChannelNumber);
                if (!channel || !channel->HasPVRChannelInfoTag())
                  return false;
                
                CApplicationMessenger::Get().SendAction(CAction(ACTION_CHANNEL_SWITCH, (float)iChannelNumber), WINDOW_INVALID, false);
              }
            }
          }
        }
        else
        {
          // filesystem provider like slingbox, cmyth, etc
          int iChannelNumber = -1;
          std::string strChannel = StringUtils::Format("%i", action.GetID() - REMOTE_0);
          if (CGUIDialogNumeric::ShowAndGetNumber(strChannel, g_localizeStrings.Get(19000)))
            iChannelNumber = atoi(strChannel.c_str());
          
          if (iChannelNumber > 0)
            CApplicationMessenger::Get().SendAction(CAction(ACTION_CHANNEL_SWITCH, (float)iChannelNumber), WINDOW_INVALID, false);
        }
      }
      return true;
    }
    break;
  }
  
  return false;
}