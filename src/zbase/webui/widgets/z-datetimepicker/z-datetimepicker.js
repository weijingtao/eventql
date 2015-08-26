/**
  * require_module: "z-calendar"
  * require_module: "z-button"
 **/

var DateTimePicker = function(input) {
  var tpl = $.getTemplate("widgets/z-datetimepicker", "z-datetimepicker-base-tpl");
  var widget;

  if (input.tagName == "Z-INPUT") {
    input = input.querySelector("input");
  }

  // no input provided
  if (input.tagName != "INPUT") {
    console.log("DateTimePicker Error: no input provided");
    return;
  }

  this.hide = function() {
    widget.removeAttribute("data-active");

    window.removeEventListener("click", this.__onWindowClick, false);
  };


  this.show = function() {
    this.__onTimeChange();
    var pos = input.getBoundingClientRect();

    // set widget top and left position
    widget.style.top = (pos.top + pos.height) + "px";
    widget.setAttribute("data-active", "active");
    widget.style.left = (pos.left - (widget.offsetWidth - pos.width) / 2) + "px";

    var _this = this;
    this.__onWindowClick = function() {
      _this.hide();
    };

    window.addEventListener("click", this.__onWindowClick, false);
  };


  /*************************** PRIVATE **********************************/

  this.__init = function() {
    // insert tpl after input
    input.style.cursor = "pointer";
    input.parentNode.insertBefore(tpl, input.nextSibling);
    //FIXME
    widget = input.nextElementSibling;
    var _this = this;

    // don't close on click within datetimepicker
    widget.addEventListener("click", function(e) {
      e.stopPropagation();
    }, false);

    widget.querySelector("button").addEventListener("click", function(e) {
      _this.__apply();
    }, false);

    this.__handleVisibility();
    this.__controlTimeInput();
  };


  this.__handleVisibility = function() {
    var _this = this;

    input.addEventListener("click", function(e) {
      e.stopPropagation();
      if (widget.hasAttribute("data-active")) {
        _this.hide();
      } else {
        _this.show();
      }
    }, false);
  };


  this.__controlTimeInput = function() {
    var inputs = widget.querySelectorAll("input");
    var _this = this;

    for (var i = 0; i < inputs.length; i++) {
      inputs[i].addEventListener("blur", function() {

        switch (this.value.length) {
          // if no value is given set it to 00
          case 0:
            this.value = "00";
            break;

          case 1:
            var value = parseInt(this.value, 10);
            // non-integer value
            if (isNaN(value)) {
              _this.__renderTimeControlError(this);
              return;
            }
            this.value = Fnord.appendLeadingZero(value);
            break;

          case 2:
            // non-integer value
            if (isNaN(parseInt(this.value[0], 10)) ||
                isNaN(parseInt(this.value[1], 10))) {
              _this.__renderTimeControlError(this);
              return;
            }
            break;
        }

        var value = parseInt(this.value, 10);
        if (this.getAttribute("data-factor") == "3600") {
          // hours value > 24
          if (value > 23) {
            _this.__renderTimeControlError(this);
            return;
          }
        } else {
          // minutes or seconds value > 59
          if (value > 59) {
            _this.__renderTimeControlError(this);
            return;
          }
        }

        // correct input value
        _this.__removeTimeControlError(this);
      }, false);
    }
  };


  this.__removeTimeControlError = function(time_control) {
    time_control.classList.remove("error");

    // if only correct input values enable apply button
    if (widget.querySelector("input.error") == null) {
      widget.querySelector("button").removeAttribute("data-state");
    }
  };


  this.__renderTimeControlError = function(time_control) {
    time_control.classList.add("error");

    // disable apply button
    widget.querySelector("button").setAttribute("data-state", "disabled");
  };


  this.__renderErrorMessage = function() {
    widget.querySelector("z-datetimepicker-error")
      .setAttribute("data-active", "active");
  };


  this.__removeErrorMessage = function() {
    widget.querySelector("z-datetimepicker-error")
        .removeAttribute("data-active", "active");
  };


  // set time value and human formatted time string
  this.__onTimeChange = function() {
    var timestamp = Math.floor(
      parseInt(input.getAttribute("data-timestamp"), 10));

    // set input value as human formatted string
    input.value = DateUtil.printTimestamp(timestamp);

    timestamp = Math.floor(timestamp / 1000);
    var date = new Date(timestamp);
    var utc_day_start = DateUtil.getStartOfDay(timestamp) +
      date.getTimezoneOffset() * DateUtil.millisPerMinute;

    // set z-calendar selection
    widget.querySelector("z-calendar").setAttribute("data-selected", utc_day_start);
    // set hours input value
    widget.querySelector("input[data-value='hours']").value =
      DateUtil.appendLeadingZero(date.getHours());
    // set minutes input value
    widget.querySelector("input[data-value='minutes']").value =
      DateUtil.appendLeadingZero(date.getMinutes());
    // set seconds input value
    widget.querySelector("input[data-value='seconds']").value =
      DateUtil.appendLeadingZero(date.getSeconds());
  };


  // get selected datetime as timestamp
  this.__getTimeValue = function() {
    var timestamp = DateUtil.parseTimestamp(
      parseInt(widget.querySelector("z-calendar")
        .getAttribute("data-selected"), 10));

    var inputs = widget.querySelectorAll("input");
    for (var i = 0; i < inputs.length; i++) {
      var value = parseInt(inputs[i].value, 10);
      timestamp += value * parseInt(inputs[i].getAttribute("data-factor"), 10);
    }

    timestamp = timestamp * 1000;
    return timestamp;
  };


  // checks if the selected date is in the past
  this.__isValidTimeValue = function(timestamp) {
    // error -> future datetime
    if (Math.floor(timestamp / 1000) > Date.now()) {
      this.__renderErrorMessage();
      return false;
    }

    this.__removeErrorMessage();
    return true;
  };


  this.__apply = function() {
    var timestamp = this.__getTimeValue();

    // invalid timestamp
    if (!this.__isValidTimeValue(timestamp)) {
      return;
    }

    input.setAttribute("data-timestamp", timestamp);
    this.__onTimeChange();
    this.hide();

    // fire change event on input
    var evt = new CustomEvent("z-datetimepicker-change");
    input.dispatchEvent(evt);
  };


  // init
  this.__init();
};
