//
//  ViewController.mm
//  quadapult
//
//  Created by Anthony Thibault on 1/28/12.
//  Copyright (c) 2012 __MyCompanyName__. All rights reserved.
//

#import "ViewController.h"

#include "quadapult.h"

@interface ViewController () {

}
@property (strong, nonatomic) EAGLContext *context;

- (void)setupGL;
- (void)tearDownGL;

@end

@implementation ViewController

@synthesize context = _context;
//@synthesize effect = _effect;

- (void)dealloc
{
    [_context release];
//    [_effect release];
    [super dealloc];
}

- (void)viewDidLoad
{
    [super viewDidLoad];

    self.context = [[[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES1] autorelease];

    if (!self.context) {
        NSLog(@"Failed to create ES context");
    }

    GLKView *view = (GLKView *)self.view;
    view.context = self.context;
    view.drawableDepthFormat = GLKViewDrawableDepthFormat24;

    [self setupGL];
}

- (void)viewDidUnload
{
    [super viewDidUnload];

    [self tearDownGL];

    if ([EAGLContext currentContext] == self.context) {
        [EAGLContext setCurrentContext:nil];
    }
    self.context = nil;
}

- (void)didReceiveMemoryWarning
{
    [super didReceiveMemoryWarning];
    // Release any cached data, images, etc. that aren't in use.
}

- (BOOL)shouldAutorotateToInterfaceOrientation:(UIInterfaceOrientation)interfaceOrientation
{
    // Return YES for supported orientations
    if ([[UIDevice currentDevice] userInterfaceIdiom] == UIUserInterfaceIdiomPhone) {
        return (interfaceOrientation != UIInterfaceOrientationPortraitUpsideDown);
    } else {
        return YES;
    }
}

- (void)setupGL
{
    [EAGLContext setCurrentContext:self.context];

    // get the appPath
    CFStringRef homeDir = (CFStringRef)NSHomeDirectory();

    char appPath[1024];
    CFStringGetCString(homeDir, appPath, 1024, kCFStringEncodingASCII);
    strcat(appPath, "/quadapult.app/");

    QUADAPULT_Init(appPath);
}

- (void)tearDownGL
{
    [EAGLContext setCurrentContext:self.context];

    QUADAPULT_Shutdown();
}

#pragma mark - GLKView and GLKViewController delegate methods

- (void)update
{
    QUADAPULT_Update(self.timeSinceLastUpdate);
}

- (void)glkView:(GLKView *)view drawInRect:(CGRect)rect
{
    QUADAPULT_Draw();
}

@end
