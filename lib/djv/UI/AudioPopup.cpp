// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#include <djv/UI/AudioPopup.h>

#include <djv/Models/AudioModel.h>

#include <djv/UI/ValueSlider.h>
#include <ftk/UI/RowLayout.h>
#include <ftk/UI/ToolButton.h>

namespace djv
{
    namespace ui
    {
        struct AudioPopup::Private
        {
            std::shared_ptr<ftk::ToolButton> muteButton;
            std::shared_ptr<djv::ui::IntValueSlider> volumeSlider;

            std::shared_ptr<ftk::Observer<bool> > muteObserver;
            std::shared_ptr<ftk::Observer<float> > volumeObserver;
        };

        void AudioPopup::_init(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<models::AudioModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            IWidgetPopup::_init(
                context,
                "djv::ui::AudioPopup",
                parent);
            FTK_P();

            p.muteButton = ftk::ToolButton::create(context);
            p.muteButton->setIcon("Mute");
            p.muteButton->setCheckable(true);
            p.muteButton->setTooltip("Mute the audio");

            p.volumeSlider = djv::ui::IntValueSlider::create(context);
            p.volumeSlider->setDefault(100);
            p.volumeSlider->setRange(0, 100);
            p.volumeSlider->setStep(1);
            p.volumeSlider->setLargeStep(10);
            p.volumeSlider->setTooltip("Audio volume");

            auto layout = ftk::HorizontalLayout::create(context);
            layout->setMarginRole(ftk::SizeRole::MarginSmall);
            layout->setSpacingRole(ftk::SizeRole::SpacingSmall);
            p.volumeSlider->setParent(layout);
            p.muteButton->setParent(layout);
            setWidget(layout);

            p.muteButton->setCheckedCallback(
                [model](bool value)
                {
                    model->setMute(value);
                });

            p.volumeSlider->setCallback(
                [model](int value)
                {
                    model->setVolume(value / 100.F);
                });

            p.muteObserver = ftk::Observer<bool>::create(
                model->observeMute(),
                [this](bool value)
                {
                    _p->muteButton->setChecked(value);
                });

            p.volumeObserver = ftk::Observer<float>::create(
                model->observeVolume(),
                [this](float value)
                {
                    _p->volumeSlider->setValue(std::roundf(value * 100.F));
                });
        }

        AudioPopup::AudioPopup() :
            _p(new Private)
        {}

        AudioPopup::~AudioPopup()
        {}

        std::shared_ptr<AudioPopup> AudioPopup::create(
            const std::shared_ptr<ftk::Context>& context,
            const std::shared_ptr<models::AudioModel>& model,
            const std::shared_ptr<IWidget>& parent)
        {
            auto out = std::shared_ptr<AudioPopup>(new AudioPopup);
            out->_init(context, model, parent);
            return out;
        }
    }
}
