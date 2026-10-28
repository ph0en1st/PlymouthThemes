/* Rootless, headless preview using the installed Plymouth script plugin.
 * A memory-only renderer replaces hardware access, not Plymouth's script engine.
 * Never starts plymouthd or opens a VT/DRM device. Development tool only.
 */
#include <assert.h>
#include <dlfcn.h>
#include <png.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>
#include <ply-boot-splash-plugin.h>
#include <ply-pixel-buffer.h>
#include <ply-renderer.h>
#include <ply-logger.h>

static ply_pixel_buffer_t *pixels;
static const ply_boot_splash_plugin_interface_t *api;
static ply_boot_splash_plugin_t *plugin;
static const char *output;
static int width, height, tick, motion;
static unsigned char renderer_token, head_token;

/* The real pixel-display/sprite/image libraries draw into this pixel buffer. */
ply_pixel_buffer_t *ply_renderer_get_buffer_for_head(ply_renderer_t *r, ply_renderer_head_t *h) {
    (void)r; (void)h; return pixels;
}
void ply_renderer_flush_head(ply_renderer_t *r, ply_renderer_head_t *h) {(void)r; (void)h;}
bool ply_renderer_get_capslock_state(ply_renderer_t *r) {(void)r; return false;}
const char *ply_renderer_get_keymap(ply_renderer_t *r) {(void)r; return "us";}
bool ply_renderer_get_panel_properties(ply_renderer_t *r, int *w, int *h, ply_pixel_buffer_rotation_t *rot, int *scale) {
    (void)r; (void)w; (void)h; (void)rot; (void)scale; return false;
}
static void snapshot(const char *name) {
    char filename[4096];
    snprintf(filename,sizeof filename,"%s/%dx%d-%s.png",output,width,height,name);
    uint32_t *argb=ply_pixel_buffer_get_argb32_data(pixels);
    unsigned char *rgb=malloc((size_t)width*height*3); assert(rgb);
    for (size_t i=0;i<(size_t)width*height;i++) {
        rgb[i*3]=(argb[i]>>16)&255; rgb[i*3+1]=(argb[i]>>8)&255; rgb[i*3+2]=argb[i]&255;
    }
    png_image image={0};image.version=PNG_IMAGE_VERSION;
    image.flags=PNG_IMAGE_FLAG_FAST;
    image.width=width;image.height=height;image.format=PNG_FORMAT_RGB;
    assert(png_image_write_to_file(&image,filename,0,rgb,0,NULL)); free(rgb);
}
static void step(void *data, ply_event_loop_t *loop) {
    (void)data; tick++;
    if(motion) {
        struct timespec before, after;
        clock_gettime(CLOCK_MONOTONIC, &before);
        char name[32]; snprintf(name,sizeof name,"motion-%03d",tick);
        snapshot(name);
        if(tick==255) {api->hide_splash_screen(plugin,loop);ply_event_loop_exit(loop,0);return;}
        clock_gettime(CLOCK_MONOTONIC, &after);
        double elapsed=(after.tv_sec-before.tv_sec)+(after.tv_nsec-before.tv_nsec)/1e9;
        double delay=1.0/30.0-elapsed;
        ply_event_loop_watch_for_timeout(loop,delay>0?delay:0.001,step,NULL);return;
    }
    if(tick==10) {snapshot("boot");api->on_boot_progress(plugin,1,0.5);}
    if(tick==20) snapshot("drivers");
    if(tick==25) api->display_password(plugin,"Unlock recovery volume / Разблокировка",0);
    if(tick==26) snapshot("password-empty");
    if(tick==27) api->display_password(plugin,"Unlock recovery volume / Разблокировка",12);
    if(tick==30) snapshot("password");
    if(tick==31) api->display_password(plugin,"Unlock recovery volume",80);
    if(tick==33) snapshot("password-long");
    if(tick==34) api->display_password(plugin,"Unlock recovery volume",2);
    if(tick==35) snapshot("password-backspace");
    if(tick==36) api->display_question(plugin,"Continue recovery? / Продолжить?","yes / да");
    if(tick==40) snapshot("question");
    if(tick==41) api->display_question(plugin,"A long question", "a deliberately long answer that should fit inside the entry field without escaping its border");
    if(tick==43) snapshot("question-long");
    if(tick==44) {api->display_normal(plugin);api->display_message(plugin,"ERROR: recovery volume unavailable / Том недоступен");}
    if(tick==47) snapshot("error");
    if(tick==48) api->hide_message(plugin,"ERROR: recovery volume unavailable / Том недоступен");
    if(tick==50) snapshot("normal-restored");
    if(tick==51 && api->display_prompt) api->display_prompt(plugin,"Modern secret prompt", "NEVER_RENDER_THIS_SECRET",true);
    if(tick==54) snapshot("modern-secret");
    if(tick==55 && api->display_prompt) api->display_prompt(plugin,"Modern question", "yes / да",false);
    if(tick==58) snapshot("modern-question");
    if(tick==59) api->display_normal(plugin);
    if(tick==75) snapshot("cycle-end");
    if(tick==85) snapshot("cycle-restart");
    if(tick==87) api->hide_splash_screen(plugin,loop);
    if(tick==89) {snapshot("handoff");ply_event_loop_exit(loop,0);return;}
    ply_event_loop_watch_for_timeout(loop,0.1,step,NULL);
}
int main(int argc,char **argv) {
    if(argc!=5 && argc!=6) {fprintf(stderr,"Usage: %s preview.plymouth OUTPUT WIDTH HEIGHT [--motion]\n",argv[0]);return 2;}
    motion=argc==6 && strcmp(argv[5],"--motion")==0;
    output=argv[2];width=atoi(argv[3]);height=atoi(argv[4]);assert(width>0&&height>0);
    ply_toggle_tracing();
    pixels=ply_pixel_buffer_new(width,height);assert(pixels);
    void *module=dlopen("/usr/lib/plymouth/script.so",RTLD_NOW|RTLD_GLOBAL);
    if(!module){fprintf(stderr,"%s\n",dlerror());return 1;}
    const ply_boot_splash_plugin_interface_t *(*get_api)(void)=dlsym(module,"ply_boot_splash_plugin_get_interface");assert(get_api);
    api=get_api();ply_key_file_t *key=ply_key_file_new(argv[1]);assert(ply_key_file_load(key));
    plugin=api->create_plugin(key);assert(plugin);
    ply_pixel_display_t *display=ply_pixel_display_new((ply_renderer_t*)&renderer_token,(ply_renderer_head_t*)&head_token);assert(display);
    api->add_pixel_display(plugin,display);
    ply_event_loop_t *loop=ply_event_loop_get_default();
    ply_buffer_t *boot=ply_buffer_new();
    assert(api->show_splash_screen(plugin,loop,boot,PLY_BOOT_SPLASH_MODE_BOOT_UP));
    ply_event_loop_watch_for_timeout(loop,0.1,step,NULL);
    int result=ply_event_loop_run(loop);
    api->remove_pixel_display(plugin,display);
    api->destroy_plugin(plugin);
    ply_pixel_display_free(display);ply_pixel_buffer_free(pixels);
    ply_key_file_free(key);ply_buffer_free(boot);
    return result;
}
