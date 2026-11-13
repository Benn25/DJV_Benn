// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the DJV project.

#pragma once

#include <djv/UI/Export.h>

#include <ftk/UI/DoubleModel.h>
#include <ftk/UI/FloatModel.h>
#include <ftk/UI/IMouseWidget.h>
#include <ftk/UI/IntModel.h>
#include <ftk/UI/SliderPopup.h>
#include <ftk/Core/Observable.h>

#include <cmath>
#include <functional>
#include <type_traits>

namespace djv
{
    namespace ui
    {
        //! A number slider as Blender draws and works one: the value written
        //! inside the bar that fills to it, in the room an edit field and a
        //! slider beside it took.
        //!
        //! - Drag to change it: across the slider is across the range. Shift
        //!   is ten times finer, Control goes by steps.
        //! - Click without dragging, or press Return over it, to type a value;
        //!   Return keeps it, Escape leaves it as it was.
        //! - Over it: Backspace puts the default back, Minus negates, Control
        //!   and the wheel step it, Control+C and Control+V copy and paste.
        //! - Right click for the reset and range options.
        class DJV_UI_API_TYPE IValueSlider : public ftk::IMouseWidget
        {
            FTK_NON_COPYABLE(IValueSlider);

        protected:
            DJV_UI_API void _init(
                const std::shared_ptr<ftk::Context>&,
                const std::string& objectName,
                const std::shared_ptr<IWidget>& parent);

            DJV_UI_API IValueSlider();

        public:
            DJV_UI_API virtual ~IValueSlider() = 0;

            //! Get the digits after the point; none for a whole number.
            DJV_UI_API int getPrecision() const;

            //! Set the digits after the point.
            DJV_UI_API void setPrecision(int);

            DJV_UI_API ftk::Size2I getSizeHint() const override;
            DJV_UI_API void setGeometry(const ftk::Box2I&) override;
            DJV_UI_API void setEnabled(bool) override;
            DJV_UI_API void sizeHintEvent(const ftk::SizeHintEvent&) override;
            DJV_UI_API void drawEvent(const ftk::Box2I&, const ftk::DrawEvent&) override;
            DJV_UI_API void mouseLeaveEvent() override;
            DJV_UI_API void mouseMoveEvent(ftk::MouseMoveEvent&) override;
            DJV_UI_API void mousePressEvent(ftk::MouseClickEvent&) override;
            DJV_UI_API void mouseReleaseEvent(ftk::MouseClickEvent&) override;
            DJV_UI_API void scrollEvent(ftk::ScrollEvent&) override;
            DJV_UI_API void keyPressEvent(ftk::KeyEvent&) override;
            DJV_UI_API void keyReleaseEvent(ftk::KeyEvent&) override;

        protected:
            virtual double _getValue() const = 0;
            //! Set from the user: a value typed may widen a soft range, one
            //! dragged keeps to it.
            virtual void _setValue(double, bool typed) = 0;
            virtual double _getMin() const = 0;
            virtual double _getMax() const = 0;
            virtual double _getStep() const = 0;
            virtual bool _hasDefault() const = 0;
            virtual double _getDefault() const = 0;
            virtual bool _isInteger() const = 0;
            virtual void _pressed(bool) = 0;
            virtual std::shared_ptr<ftk::IWidgetPopup> _createPopup() = 0;

            DJV_UI_API bool _isDragging() const;

        private:
            std::string _format(double) const;
            void _setDragged(double);
            void _editBegin();
            void _editEnd(bool keep);
            void _showPopup();

            FTK_PRIVATE();
        };

        //! A number slider of a value type and its ftk model, with the
        //! interface of the ftk edit slider it stands in for.
        template<typename T, typename TModel, typename TRange, typename TPopup>
        class ValueSlider : public IValueSlider
        {
        protected:
            void _init(
                const std::shared_ptr<ftk::Context>& context,
                const std::shared_ptr<TModel>& model,
                const std::shared_ptr<IWidget>& parent)
            {
                IValueSlider::_init(context, "djv::ui::ValueSlider", parent);
                _model = model;
                if (std::is_integral<T>::value)
                {
                    setPrecision(0);
                }
                // As the ftk sliders do: every change, the user's or not.
                _valueObserver = ftk::Observer<T>::create(
                    _model->observeValue(),
                    [this](T value)
                    {
                        setDrawUpdate();
                        if (_callback)
                        {
                            _callback(value);
                        }
                        if (_pressedCallback)
                        {
                            _pressedCallback(value, _isDragging());
                        }
                    });
                _rangeObserver = ftk::Observer<TRange>::create(
                    _model->observeRange(),
                    [this](const TRange&)
                    {
                        setDrawUpdate();
                    });
            }

            ValueSlider() = default;

        public:
            virtual ~ValueSlider() {}

            //! Create a new widget.
            static std::shared_ptr<ValueSlider> create(
                const std::shared_ptr<ftk::Context>& context,
                const std::shared_ptr<IWidget>& parent = nullptr)
            {
                return create(context, TModel::create(), parent);
            }

            //! Create a new widget.
            static std::shared_ptr<ValueSlider> create(
                const std::shared_ptr<ftk::Context>& context,
                const std::shared_ptr<TModel>& model,
                const std::shared_ptr<IWidget>& parent = nullptr)
            {
                auto out = std::shared_ptr<ValueSlider>(new ValueSlider);
                out->_init(context, model, parent);
                return out;
            }

            T getValue() const { return _model->getValue(); }
            void setValue(T value) { _model->setValue(value); }
            void setCallback(const std::function<void(T)>& value) { _callback = value; }
            void setPressedCallback(const std::function<void(T, bool)>& value) { _pressedCallback = value; }
            const TRange& getRange() const { return _model->getRange(); }
            void setRange(const TRange& value) { _model->setRange(value); }
            void setRange(T min, T max) { _model->setRange(TRange(min, max)); }
            T getStep() const { return _model->getStep(); }
            void setStep(T value) { _model->setStep(value); }
            T getLargeStep() const { return _model->getLargeStep(); }
            void setLargeStep(T value) { _model->setLargeStep(value); }
            T getDefault() const { return _model->getDefault(); }
            void setDefault(T value) { _model->setDefault(value); }
            const std::shared_ptr<TModel>& getModel() const { return _model; }

        protected:
            double _getValue() const override
            {
                return static_cast<double>(_model->getValue());
            }

            void _setValue(double value, bool typed) override
            {
                const T t = std::is_integral<T>::value ?
                    static_cast<T>(std::llround(value)) :
                    static_cast<T>(value);
                if (typed)
                {
                    _model->setValueSoft(t);
                }
                else
                {
                    _model->setValue(t);
                }
            }

            double _getMin() const override { return static_cast<double>(_model->getRange().min()); }
            double _getMax() const override { return static_cast<double>(_model->getRange().max()); }
            double _getStep() const override { return static_cast<double>(_model->getStep()); }
            bool _hasDefault() const override { return _model->hasDefault(); }
            double _getDefault() const override { return static_cast<double>(_model->getDefault()); }
            bool _isInteger() const override { return std::is_integral<T>::value; }

            void _pressed(bool value) override
            {
                if (_pressedCallback)
                {
                    _pressedCallback(_model->getValue(), value);
                }
            }

            std::shared_ptr<ftk::IWidgetPopup> _createPopup() override
            {
                if (auto context = getContext())
                {
                    return TPopup::create(context, _model);
                }
                return nullptr;
            }

        private:
            std::shared_ptr<TModel> _model;
            std::shared_ptr<ftk::Observer<T> > _valueObserver;
            std::shared_ptr<ftk::Observer<TRange> > _rangeObserver;
            std::function<void(T)> _callback;
            std::function<void(T, bool)> _pressedCallback;
        };

        typedef ValueSlider<float, ftk::FloatModel, ftk::RangeF, ftk::FloatSliderPopup> FloatValueSlider;
        typedef ValueSlider<double, ftk::DoubleModel, ftk::RangeD, ftk::DoubleSliderPopup> DoubleValueSlider;
        typedef ValueSlider<int, ftk::IntModel, ftk::RangeI, ftk::IntSliderPopup> IntValueSlider;
    }
}
