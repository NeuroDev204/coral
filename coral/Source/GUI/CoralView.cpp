/*
Coral
Copyright (C) 2025  Coral

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU Affero General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Affero General Public License for more details.

You should have received a copy of the GNU Affero General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "CoralView.h"
#include "CoralController.h"
#include "CoralSettingsDialog.h"
#include "CoralPresetExportDialog.h"
#include "CoralPresetImportDialog.h"

CoralView::CoralView()
{
	CoralModel::getModel().addListener(this);

	preset_list_.setJustificationType(Justification::centredLeft);
	preset_list_.setTextWhenNoChoicesAvailable(L"");
	preset_list_.setSize(LIST_WIDTH, LIST_HEIGHT);
	preset_list_.setDescription(TRANS("Preset List"));
	preset_list_.setWantsKeyboardFocus(true);
	addAndMakeVisible(&preset_list_);
	preset_list_.addListener(this);
	preset_list_.addMouseListener(this, true);

	endpoint_list_.setJustificationType(Justification::centredLeft);
	endpoint_list_.setTextWhenNoChoicesAvailable(L"");
	endpoint_list_.setSize(LIST_WIDTH, LIST_HEIGHT);
	endpoint_list_.setDescription(TRANS("Playback Device List"));
	endpoint_list_.setWantsKeyboardFocus(true);
	addAndMakeVisible(&endpoint_list_);
	endpoint_list_.addListener(this);
	endpoint_list_.addMouseListener(this, true);
	endpoint_list_.onShowPopup = []() {
		CoralController::getInstance().checkDeviceChanges();
		};

    addChildComponent(&error_notification_);
}

CoralView::~CoralView()
{
	CoralModel::getModel().removeListener(this);
}

void CoralView::showErrorNotification(bool show)
{
    if (show)
    {
        error_notification_.setMessage(TRANS("Coral is unable to play processed audio through the selected output device.\nAnother application could be using it in exclusive mode or the device could be\ndisconnected. To disable exclusive mode follow these "),
            { TRANS("steps."), "https://github.com/NeuroDev204/coral/issues" });
        auto component_bounds = endpoint_list_.getBounds();
        auto x = component_bounds.getX() - (CoralNotification::MAX_WIDTH - component_bounds.getWidth());
        component_bounds.setX(x);
        component_bounds.setY(component_bounds.getBottom() + 5);
        component_bounds.setWidth(CoralNotification::MAX_WIDTH);
        component_bounds.setHeight(CoralNotification::MAX_HEIGHT);
        error_notification_.setBounds(component_bounds);
        error_notification_.showMessage(false);
    }
    else
    {
        error_notification_.setVisible(false);
    }
}

void CoralView::comboBoxChanged(ComboBox* combobox)
{
	auto index = combobox->getSelectedItemIndex();
	if (index >= 0 && index < combobox->getNumItems())
	{
		if (combobox == &preset_list_)
		{
			CoralController::getInstance().setPreset(index);
		}
		else if (combobox == &endpoint_list_)
		{
			CoralController::getInstance().setOutput(index, false);
		}
	}
}

void CoralView::modelChanged(CoralModel::Event model_event)
{
	if (model_event == CoralModel::Event::OutputListUpdated)
	{
		auto output_devices = CoralModel::getModel().getOutputDevices();

		endpoint_list_.clear(NotificationType::dontSendNotification);
		int id = 1;
		for (auto device : output_devices)
		{
			endpoint_list_.addItem(device.deviceFriendlyName.c_str(), id);
			if (device.deviceNumChannel < 2)
			{
				endpoint_list_.setItemEnabled(id, false);
			}

			id++;
		}
	}

	if (model_event == CoralModel::Event::OutputSelected)
	{
		endpoint_list_.setSelectedId(CoralModel::getModel().getSelectedOutputIndex() + 1, NotificationType::dontSendNotification);

        if (endpoint_list_.getError())
        {
            showErrorNotification(true);
        }
        else
        {
            showErrorNotification(false);
        }
	}

    if (model_event == CoralModel::Event::OutputError)
    {
        if (!CoralController::getInstance().isPlaybackDeviceAvailable())
        {
            endpoint_list_.setItemEnabled(CoralModel::getModel().getSelectedOutputIndex() + 1, false);
            endpoint_list_.setError(true);
        }
        else
        {
            endpoint_list_.setItemEnabled(CoralModel::getModel().getSelectedOutputIndex() + 1, true);
            endpoint_list_.setError(false);
            error_notification_.setVisible(false);
        }
    }
	
	if (model_event == CoralModel::Event::PresetSelected)
	{
		preset_list_.setSelectedId(CoralModel::getModel().getSelectedPreset() + 1, NotificationType::dontSendNotification);
	}

	if (model_event == CoralModel::Event::PresetListUpdated)
	{
		preset_list_.clear(NotificationType::dontSendNotification);
		
		auto count = CoralModel::getModel().getPresetCount();
		auto preset_type = CoralModel::PresetType::AppPreset;
		for (auto i=0; i<count; i++)
		{
			auto preset = CoralModel::getModel().getPreset(i);
			if (preset_type != preset.type)
			{
				preset_list_.addSeparator();
				preset_type = preset.type;
			}

			auto name = preset.modified ? preset.name + L" *" : preset.name;
			preset_list_.addItem(name, i+1);
		}

		preset_list_.setSelectedId(CoralModel::getModel().getSelectedPreset() + 1, NotificationType::dontSendNotification);
	}

	if (model_event == CoralModel::Event::PresetModified)
	{
		auto& model = CoralModel::getModel();
		auto selected = model.getSelectedPreset();
		auto preset = model.getPreset(selected);
		if (preset.name.isEmpty())
		{
			return;
		}
		
		if (model.isPresetModified(selected))
		{
			preset_list_.changeItemText(selected + 1, preset.name + L" *");
			if (!preset_list_.isPopupActive())
			{
				preset_list_.setText(preset.name + L" *", NotificationType::dontSendNotification);
			}
		}
		else
		{
			preset_list_.changeItemText(selected + 1, preset.name);
			if (!preset_list_.isPopupActive())
			{
				preset_list_.setText(preset.name, NotificationType::dontSendNotification);
			}
		}
		
		return;
	}
}

void CoralView::mouseEnter(const MouseEvent&)
{
	preset_list_.highlightText(preset_list_.isMouseOver(true));
	endpoint_list_.highlightText(endpoint_list_.isMouseOver(true));

    if (endpoint_list_.getError() && (endpoint_list_.isMouseOver(true) || error_notification_.isMouseOver(true)))
    {   
        showErrorNotification(true);
    }
    else 
    {
        showErrorNotification(false);
    }
}

void CoralView::mouseExit(const MouseEvent&)
{
	preset_list_.highlightText(preset_list_.isMouseOver(true));
	endpoint_list_.highlightText(endpoint_list_.isMouseOver(true));
}