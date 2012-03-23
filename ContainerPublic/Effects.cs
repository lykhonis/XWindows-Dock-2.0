using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Windows.Media.Effects;
using System.Windows;
using System.Windows.Media;

namespace ContainerPublic
{
    public class FadeInEffect : ShaderEffect
    {
        public static DependencyProperty InputProperty = RegisterPixelShaderSamplerProperty("Input", typeof(FadeInEffect), 0);

        public static DependencyProperty DeltaProperty = DependencyProperty.Register("Delta", typeof(double),
            typeof(FadeInEffect), new PropertyMetadata(new double(), PixelShaderConstantCallback(0)));

        public FadeInEffect()
        {
            PixelShader = new PixelShader { UriSource = new Uri("FadeInEffect.ps", UriKind.Relative) };

            UpdateShaderValue(InputProperty);
            UpdateShaderValue(DeltaProperty);
        }

        public virtual Brush Input
        {
            get
            {
                return ((Brush)(GetValue(InputProperty)));
            }
            set
            {
                SetValue(InputProperty, value);
            }
        }

        public virtual double Delta
        {
            get
            {
                return ((double)(GetValue(DeltaProperty)));
            }
            set
            {
                SetValue(DeltaProperty, value);
            }
        }
    }
}
