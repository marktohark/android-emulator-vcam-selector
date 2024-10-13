<template>
  <n-modal v-model:show="sw">
    <n-card
        style="width: 600px;height: 500px;overflow: auto;"
        size="huge"
        role="dialog"
        aria-modal="true"
    >
      <div><b>Out：</b></div>
      <div style="margin-bottom: 10px"><pre>{{stdout}}</pre></div>
      <div><b>Err：</b></div>
      <div style="margin-bottom: 10px"><pre>{{stderr}}</pre></div>
    </n-card>
  </n-modal>
</template>

<script setup>
import {ref, defineExpose} from 'vue'

const stdout = ref("")
const stderr = ref("")
const sw = ref(false)

const open = (out, err) => {
  stdout.value = out
  stderr.value = err
  sw.value = true
}

defineExpose({
  open,
})
</script>

<style scoped>

</style>