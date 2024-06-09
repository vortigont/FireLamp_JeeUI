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
  ws.send_post("set_wcfg_alrm", newdata);
}

// add our fuction to custom funcs that could be called for js_func frames
customFuncs["alarm_items_load"] = alarm_items_load;
customFuncs["alarm_item_set"] = alarm_item_set;
