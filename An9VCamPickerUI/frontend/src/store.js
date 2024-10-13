import { defineStore } from 'pinia'
import {LsCamera} from "../wailsjs/go/main/App.js";
export const usGlobalStore = defineStore('global', () => {
    const ConnLs = ref([])
    const CameraList = ref([])

    const UpdateCameraList = async () => {
        let names = await LsCamera()
        CameraList.value = [{label: "Default", value: ""}]
        for (let i = 0; i < names.length; i++) {
            CameraList.value.push({label: names[i], value: names[i]})
        }
    }

    window.runtime.EventsOn("new_connection", (pid, pName) => {
        console.log("new_connection", pid, pName)
    })
    window.runtime.EventsOn("selected_vcam", (pid, vcamName) => {
        console.log("selected_vcam", pid, vcamName)
        let idx = ConnLs.value.findIndex((item) => item.pid === pid)
        ConnLs.value[idx].selectedVCamName = vcamName
    })
    window.runtime.EventsOn("updated_list", (row) => {
        console.log("updated_list", row)
        ConnLs.value = row
    })

    return { ConnLs, CameraList, UpdateCameraList }
})