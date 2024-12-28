// builds alarm items list based on provided array of option values from MCU
function alarm_items_load(arg){
    if (typeof arg != 'object' || typeof arg.block[0].event != 'object'){
        console.log("alarm_items_load() got wrong argument type", arg)
        return
    }
    let dataobj = arg.block[0]
    let data = {};
    data["block"] = [];
    // copy raw k:v's
    for (let key in dataobj ){
      if (typeof dataobj[key] != 'object')
        data.block.push({[key] : dataobj[key]})
    }
    // rename keys for array elements
    dataobj.event.forEach((obj, idx, array) => {
        for (let key in obj){
            data.block.push({[key+idx] : obj[key]})
        }
    });
    var r = render();
    r.value(data);
}

// get section data for alarm item submission and reconstruct the object to structure acceptable for alarm widget
function alarm_item_set(arg){
  //console.log("Alarm post arg:", arg)
  // button click may return awesome icon item
  let id = arg.target.id === "" ? arg.target.parentElement.id : arg.target.id;
  //console.log("Alarm item to set:", id)
  let form = go("#"+id), data = go.formdata(go("input, textarea, select", form));
  let idx = Object.keys(data)[0];
  idx = idx.slice(-1)
  let newdata = {}
  for (let key in data ){
    newdata[key.slice(0, key.length - 1)] = data[key]
  }
  newdata["idx"] = Number(idx)

  //console.log("Collected form newdata:", newdata)
  ws.send_post("set_mod_alrm", newdata);
}

// builds OmniCron tasks list brief UI page
function omnicron_tasks_load(arg){
  if (typeof arg != 'object' || typeof arg.block[0].event != 'object'){
      console.log("omnicron_items_load() got wrong argument type", arg)
      return
  }
  let tasks_data = arg.block[0]
  let pkg = {
    "pkg": "interface",
    "final": true,
    "section": "omnicron_tasks",
    "block": []
  };

  // make a deep copy, 'cause we'll modify the object
  let ui_obj = structuredClone(_.get(uiblocks, "lampui.sections.mod_omnicron.task_brief"));

  // rename keys for each array's elements in 'event' obj
  tasks_data.event.forEach((obj, idx, array) => {
    ui_obj.section = "omni_task" + idx
    ui_obj.block[0]["value"] = obj.active
    ui_obj.block[1]["value"] = obj.descr
    ui_obj.block[2]["value"] = idx        // edit btn
    ui_obj.block[3]["value"] = idx        // del btn
    pkg.block.push(structuredClone(ui_obj))
  });

  var r = render();
  r.make(pkg);
}

/**
 * generates UIData block with a drop-down effects selector
 * based on effect index on MCU's FS
 * translates 
 * https://dmitripavlutin.com/javascript-fetch-async-await/
 */
async function make_effect_list(arg){
  const [effidxReq, i18ndataReq] = await Promise.all([
    fetch('/effects_idx.json', {method: 'GET'}),
    fetch('/js/ui_lamp.i18n.json', {method: 'GET'})
  ])

  if (!effidxReq.ok || !i18ndataReq.ok) return;
  const effidx = await effidxReq.json();
  const i18ndata = await i18ndataReq.json();

  let efflist = {
      "id":"eff_sw_idx",
      "html":"select",
      "onChange": true,
      "value":0,
      "section":"options",
      "block":[]
  }

  effidx.forEach(
    function(v, idx, array){
      if (v.hidden) return
      efflist.block.push({"label":v.idx + ' - ' + i18ndata.ru.effNames[v.label], "value":v.idx})
    }
  )

  _.set(uiblocks, 'lampui.dynamic.efflist', efflist)
  console.log("Update effects list:", uiblocks.lampui.dynamic.efflist);

  let obj = {
    "section":"content",
    "block":[
      efflist
    ]
  }
  // update ubject on the page
  var rdr = this.rdr = render();
  rdr.content(obj)
  // request controls refresh from MCU
  ws.send_post("eff_ctrls", {});
}

async function make_effect_profile_selector_list(arg){
  if (!arg.Effect) return
  const req = await fetch('/eff/' + arg.Effect + ".json", {method: 'GET'})
  if (!req.ok) return
  const resp = await req.json();
  if (!(resp instanceof Object)) return

  //console.log("Got eff presets list:", resp);

  let presets = {
    "id":"eff_preset",
    "html":"select",
    "onChange": true,
    "section":"options",
    "block":[]
  }

  let idx = 0
  for (profile of resp.profiles){
    let lbl = profile.label === undefined ? "Profile_"+idx : profile.label
    presets.block.push({"label":lbl, "value":idx})
    ++idx
  }
  presets["value"] = arg.idx

  let obj = {
    "section":"content",
    "block":[
      presets
    ]
  }
  //console.log("Render presets list:", obj);
  // update ubject on the page
  var rdr = this.rdr = render();
  rdr.content(obj)
}

/**
 * generate UI page for Text Scrollers
 * 
 */
async function txtscroller_mk_page_main(arg){
  const req = await fetch('/txtscroll.json', {method: 'GET'})
  if (!req.ok) return
  const resp = await req.json();
  if (!(resp instanceof Object)) return

  _.set(uiblocks, 'lampui.dynamic.mod_txtscroll', resp)

  let scrolls = {
    "section":"scrolls_list",
    "label": "Потоки",
    "replace": true,
    "block":[]
  }

  // rename keys for each array's element
  resp.scrollers.forEach((obj, idx, array) => {
    // make a deep copy, 'cause we'll modify the object
    let ui_obj = structuredClone(_.get(uiblocks, "lampui.sections.mod_txtscroll.scroll_item"));
    ui_obj.section += obj.stream_id

    let index = _.findIndex(ui_obj.block, {"id":"active"})
    ui_obj.block[index]["value"] = obj.active // chk box
    //index
    ui_obj.block[_.findIndex(ui_obj.block, {"id":"descr"})]["value"] = "ID:" + obj.stream_id + " - " + obj.descr
    ui_obj.block[_.findIndex(ui_obj.block, {"id":"get_mod_txtscroll_scroll_edit"})]["value"] = obj.stream_id        // edit btn
    ui_obj.block[_.findIndex(ui_obj.block, {"id":"set_mod_txtscroll_scroll_rm"})]["value"] = obj.stream_id        // del btn
    scrolls.block.push(ui_obj)
  });

  //console.log("Render scrolls list:", scrolls);
  var rdr = this.rdr = render();
  rdr.section(scrolls)
}

// reacts on button "edit text scroll stream", creates a form with stream's config options
function txtscroller_mk_page_edit_stream(event, id, arg){
  let ui_obj = structuredClone(_.get(uiblocks, "lampui.sections.mod_txtscroll.edit_stream_form"));
  let form = ui_obj.block[0]
  // if arg == -1, then it's a new object creation
  if (arg != -1){
    let scrollers = _.get(uiblocks, "lampui.dynamic.mod_txtscroll.scrollers")
    let stream_id = _.findIndex(scrollers, {"stream_id":arg})
    let scroller = scrollers[stream_id]
    //console.log("scroller:", scroller, "sid:", stream_id);
    for (let key in scroller){
      if (typeof scroller[key] == 'object')
        continue

      let i = _.findIndex(form.block, {"id":key})
      if (i != -1)
        form.block[i]["value"] = scroller[key]
    }
    console.log("scroller:", scroller);
    console.log("form:", form);
    // set value for profiles drop-down selector
    let i = _.findIndex(ui_obj.block[0].block, {"id":"profile"})
    console.log("res:", ui_obj, " i:", i);
    ui_obj.block[0].block[i]["value"] = scroller.profile
  }

  // drop down profile selector options
  let profile_idx = _.findIndex(form.block, {"id":"profile"})
  _.get(uiblocks, "lampui.dynamic.mod_txtscroll.profiles").forEach((obj, idx, array) => {
    form.block[profile_idx].block.push( { "value":idx, "label":obj["label"] })
  });
  var rdr = this.rdr = render();
  rdr.section(ui_obj)
}

// add our fuction to custom funcs that could be called for js_func frames
customFuncs["alarm_items_load"] = alarm_items_load
customFuncs["alarm_item_set"] = alarm_item_set
customFuncs["omnicron_tasks_load"] = omnicron_tasks_load
customFuncs["make_effect_list"] = make_effect_list
customFuncs["mk_eff_profile_list"] = make_effect_profile_selector_list
customFuncs["txtscroller_mk_page_main"] = txtscroller_mk_page_main
customFuncs["txtscroller_mk_page_edit_stream"] = txtscroller_mk_page_edit_stream

// load Informer's App UIData
window.addEventListener("load", async function(ev){
	let response = await fetch("/js/ui_lamp.json", {method: 'GET'});
	if (response.ok){
		response = await response.json();
		uiblocks['lampui'] = response;
	}

}.bind(window)
);