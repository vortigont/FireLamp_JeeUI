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
  let ui_obj = JSON.parse(JSON.stringify(_.get(uiblocks, "lampui.sections.mod_omnicron.task_brief")));

  // rename keys for each array's elements in 'event' obj
  tasks_data.event.forEach((obj, idx, array) => {
    ui_obj.section = "omni_task" + idx
    ui_obj.block[0]["value"] = obj.active
    ui_obj.block[1]["value"] = obj.descr
    ui_obj.block[2]["value"] = idx        // edit btn
    ui_obj.block[3]["value"] = idx        // del btn
    pkg.block.push(JSON.parse(JSON.stringify(ui_obj)))
  });

  var r = render();
  r.make(pkg);
}


// add our fuction to custom funcs that could be called for js_func frames
customFuncs["alarm_items_load"] = alarm_items_load;
customFuncs["alarm_item_set"] = alarm_item_set;
customFuncs["omnicron_tasks_load"] = omnicron_tasks_load;
