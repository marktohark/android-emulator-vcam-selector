<template>
  <n-modal-provider>
    <div style="display: flex;gap: 10px;margin-bottom: 10px">
      <n-select v-model:value="VCmValue" :options="global.CameraList" />
      <n-button type="primary" @click="refresh">Refresh</n-button>
    </div>
    <div style="display: flex;gap: 10px;margin-bottom: 10px">
      <n-button type="info" @click="inject">Inject</n-button>
      <n-button type="warning" @click="uninject">UnInject</n-button>
    </div>

    <n-table :single-line="false">
      <thead>
      <tr>
        <th>PID</th>
        <th>Process Name</th>
        <th>Selected VCam</th>
      </tr>
      </thead>
      <tbody>
      <tr v-for="item in global.ConnLs">
        <td>{{item.pid}}</td>
        <td>{{item.pName}}</td>
        <td>{{item.selectedVCamName}}</td>
      </tr>
      </tbody>
    </n-table>
    <SetDllDialog ref="setDllDialog" />
  </n-modal-provider>
</template>

<script setup>
import {usGlobalStore} from "./store.js";
import {Apply, Inject, UnInject} from "../wailsjs/go/main/App.js";
import SetDllDialog from "./components/SetDllDialog.vue";

const global = usGlobalStore()
const setDllDialog = ref(null)

const VCmValue = ref("")
watch(VCmValue, (newVal, oldVal) => {
  apply()
})
const refresh = async () => {
  await global.UpdateCameraList()
}

const apply = async () => {
  await Apply(VCmValue.value)
}

const inject = async () => {
  const [stdout, stderr] = await Inject()
  setDllDialog.value.open(stdout, stderr)
}
const uninject = async () => {
  const [stdout, stderr] = await UnInject()
  setDllDialog.value.open(stdout, stderr)
}

onMounted(() => {
  refresh()
})
</script>

<style>

</style>
