ZBase.registerView((function() {
  var kPathPrefix = "/a/datastore/tables/";

  var load = function(path) {
    var table_id = path.substr(kPathPrefix.length);

    $.showLoader();
    $.httpGet("/api/v1/tables/" + table_id, function(r) {
      if (r.status == 200) {
        render(JSON.parse(r.response).table);
      } else {
        $.fatalError();
      }
      $.hideLoader();
    });
  };

  var render = function(schema) {
    var page = $.getTemplate(
        "views/table_editor",
        "zbase_table_editor_tpl");

    var main_menu = ZBaseMainMenu();
    main_menu.render($(".zbase_main_menu", page), kPathPrefix);

    var table_breadcrumb = $(".table_name_breadcrumb", page);
    table_breadcrumb.innerHTML = schema.name;
    table_breadcrumb.href = kPathPrefix + schema.name;

    $("z-tab.schema a", page).href = kPathPrefix + schema.name;
    $("z-tab.settings a", page).href = kPathPrefix + schema.name;

    $("z-dropdown.edit", page).addEventListener("change", function() {
      switch (this.getValue()) {
        case "add_column":
          displayAddColumnModal(schema);
          break;

        case "delete_column":
          displayDeleteColumnModal(schema);
          break;
      }
      this.setValue([]);
    }, false);


    renderTable(schema.schema.columns, $("tbody", page), "");

    $.handleLinks(page);
    $.replaceViewport(page);
  };

  var renderTable = function(columns, tbody, prefix) {
    var row_tpl = $.getTemplate(
        "views/table_editor",
        "zbase_table_editor_row_tpl");

    columns.forEach(function(column) {
      var tr = row_tpl.cloneNode(true);
      var column_name = prefix + column.name;
      $(".name", tr).innerHTML = column_name;
      $(".type", tr).innerHTML = column.type;
      $(".is_nullable", tr).innerHTML = column.optional;

      tbody.appendChild(tr);

      if (column.type == "OBJECT") {
        renderTable(column.schema.columns, tbody, column_name + ".");
      }
    });
  };

  var displayAddColumnModal = function(schema) {
    $.httpPost("/api/v1/tables/add_field?table=test&field_name=test_field&field_type=string", "", function(r) {
      console.log(r);
    });
    var modal = $(".zbase_table_editor z-modal.add_column");
    var tpl = $.getTemplate(
        "views/table_editor", 
        "zbase_table_editor_add_modal_tpl");

    var input = $("input", tpl);

    $.onClick($("button.submit", tpl), function() {
      if (input.value.length == 0) {
        $(".error_note", modal).classList.remove("hidden");
        input.classList.add("error");
        input.focus();
        return;
      }

      console.log(schema);
      return;

      var json = {
        table_name: schema.name,
        schema: {
          columns: schema.columns
        },
        update: true
      };

      json.schema.columns.push({
          name: input.value,
          type: $("z-dropdown.type", modal).getValue(),
          optional: $("z-dropdown.is_nullable", modal).getValue()});

      //TODO httpPost

    });

    $.onClick($("button.close", tpl), function() {
      modal.close();
    });

    $.replaceContent($(".container", modal), tpl);
    modal.show();
    input.focus();
  };

  var displayDeleteColumnModal = function(schema) {
    var modal = $(".zbase_table_editor z-modal.delete_column");
    var tpl = $.getTemplate(
        "views/table_editor",
        "zbase_table_editor_delete_modal_tpl");

    var input = $("input", tpl);

    $.onClick($("button.submit", tpl), function() {
      var url = "/api/v1/tables/remove_field?table=" + schema.name +
        "&field_name=" + input.value;

      $.httpPost(url, "", function(r) {
        if (r.status == 201) {
          $.navigateTo(kPathPrefix + schema.name);
        } else {
          $(".error_note", modal).classList.remove("hidden");
          $(".error_note .msg", modal).innerHTML = r.response;
          input.classList.add("error");
        }
      });

    });

    $.onClick($("button.close", tpl), function() {
      modal.close();
    });

    $.replaceContent($(".container", modal), tpl);

    modal.show();
    input.focus();
  };


  return {
    name: "table_editor",
    loadView: function(params) { load(params.path); },
    unloadView: function() {},
    handleNavigationChange: load
  };
})());
