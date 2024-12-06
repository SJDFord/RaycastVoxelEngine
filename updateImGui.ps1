Remove-Item -r -Force -erroraction silentlycontinue external/imgui
if ($error[0] -notmatch 'does not exist') {
    write-error $error[0]  # to standard error
}
git clone https://github.com/ocornut/imgui.git external/imgui